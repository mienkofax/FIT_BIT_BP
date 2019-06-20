#include <list>
#include <map>

#include <Poco/Logger.h>
#include <Poco/ScopedLock.h>
#include <Poco/NumberParser.h>

#include <Manager.h>

#include "z-wave/NotificationProcessor.h"
#include "zmq/ZMQMessage.h"

using namespace BeeeOn;
using namespace OpenZWave;
using Poco::NumberParser;

map<uint8_t, NodeInfo> NotificationProcessor::m_nodesMap;

NotificationProcessor::NotificationProcessor(set<DeviceID> &pairedDevices,
		Poco::AtomicCounter &listen):
	m_initFailed(false),
	m_pairedDevices(pairedDevices),
	m_listen(listen)
{
}

void NotificationProcessor::waitUntilQueried()
{
	m_initMutex.lock();
	m_initCondition.wait(m_initMutex);
	m_initMutex.unlock();
}

Poco::Nullable<NodeInfo> NotificationProcessor::findNodeInfo(const uint8 &nodeId)
{
	Poco::Nullable<NodeInfo> nullable;

	auto search = m_nodesMap.find(nodeId);

	if (search != m_nodesMap.end()) {
		NodeInfo &nodeI = search->second;
		nullable = nodeI;
	}

	return nullable;
}

void NotificationProcessor::valueAdded(const Notification *notification)
{
	nodeInfoMap::iterator it = m_nodesMap.find(notification->GetNodeId());

	if (it == m_nodesMap.end())
		return;

	it->second.m_values.push_back(notification->GetValueID());
}

void NotificationProcessor::valueChanged(const Notification *notification)
{
	nodeInfoMap::iterator it = m_nodesMap.find(notification->GetNodeId());
	ZWaveMessage *message;
	uint32_t manufacturer;
	uint32_t product;
	uint8_t nodeId;

	if (it == m_nodesMap.end())
		return;

	nodeId = notification->GetNodeId();

	try {
		manufacturer = NumberParser::parseHex(
			Manager::Get()->GetNodeManufacturerId(m_homeId, nodeId));
		product = NumberParser::parseHex(
			Manager::Get()->GetNodeProductId(m_homeId, nodeId));
	}
	catch (Poco::Exception &ex) {
		logger().error("failed to parse manufacturer/product value");
		logger().log(ex, __FILE__, __LINE__);
		return;
	}

	try {
		message = m_factory->create(manufacturer, product);
	}
	catch (Poco::Exception &ex) {
		logger().error("manufacturer: " + std::to_string(manufacturer)
			+ " product: " + std::to_string(product));
		logger().log(ex, __FILE__, __LINE__);
		return;
	}

	sendValue(nodeId, message, it->second.m_values);
	delete message;
}

int NotificationProcessor::sendValue(const uint8_t &nodeId, ZWaveMessage *message,
	const std::list<OpenZWave::ValueID> &values)
{
	SensorData sensorData;
	vector<ZWaveSensorValue> zwaveValues;

	for (auto &item : values) {
		string value;
		Manager::Get()->GetValueAsString(item, &value);

		zwaveValues.push_back({
			item.GetCommandClassId(),
			item.GetIndex(),
			item,
			value,
			Manager::Get()->GetValueUnits(item)});
		}

	sensorData = message->extractValues(zwaveValues);

	sensorData.setDeviceID(DeviceID(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_ZWAVE),
		message->getEUID(m_homeId, nodeId)));

	auto it = m_pairedDevices.find(sensorData.deviceID());
	if (it == m_pairedDevices.end() && !m_listen) {
		logger().warning("drop message");
		return -1;
	}

	ZMQMessage msg = ZMQMessage::fromSensorData(sensorData);
	return m_zmqClient->send(msg.toString());
}

void NotificationProcessor::valueRemoved(const Notification *notification)
{
	nodeInfoMap::iterator it = m_nodesMap.find(notification->GetNodeId());

	if (it == m_nodesMap.end())
		return;

	m_nodesMap.erase(it);
	return;
}

void NotificationProcessor::nodeAdded(const Notification *notification)
{
	NodeInfo nodeInfo;
	nodeInfo.m_polled = false;
	m_nodesMap.emplace(std::pair<uint8_t, NodeInfo>(notification->GetNodeId(), nodeInfo));

	Manager::Get()->CancelControllerCommand(notification->GetHomeId());
	Manager::Get()->WriteConfig(m_homeId);
}

void NotificationProcessor::nodeRemoved(const Notification *notification)
{
	uint8_t nodeId = notification->GetNodeId();
	nodeInfoMap::iterator it = m_nodesMap.find(nodeId);

	if (it != m_nodesMap.end())
		m_nodesMap.erase(nodeId);

	Manager::Get()->WriteConfig(m_homeId);
}

void NotificationProcessor::onNotification(const Notification *notification)
{
	Poco::Mutex::ScopedLock guard(m_lock);
	nodeInfoMap::iterator it = m_nodesMap.find(notification->GetNodeId());

	switch (notification->GetType()) {
	case Notification::Type_ValueAdded:
		valueAdded(notification);
		break;
	case Notification::Type_ValueRemoved:
		valueRemoved(notification);
		break;
	case Notification::Type_ValueChanged:
		valueChanged(notification);
		break;
	case Notification::Type_NodeAdded:
		nodeAdded(notification);
		break;
	case Notification::Type_NodeRemoved:
		nodeRemoved(notification);
		break;
	case Notification::Type_NodeEvent: {
		break;
	}
	case Notification::Type_PollingDisabled: {
		if (it != m_nodesMap.end())
			it->second.m_polled = false;

		break;
	}
	case Notification::Type_PollingEnabled: {
		if (it != m_nodesMap.end())
			it->second.m_polled = true;

		break;
	}
	case Notification::Type_DriverReady: {
		m_homeId = notification->GetHomeId();
		Manager::Get()->WriteConfig(m_homeId);
		break;
	}
	case Notification::Type_DriverFailed: {
		m_initFailed = true;

		/*
		 * Allows all waiting threads to continue their execution,
		 * after driver failed.
		 */
		m_initCondition.broadcast();

		break;
	}
	case Notification::Type_ValueRefreshed:
		break;
	case Notification::Type_AwakeNodesQueried:
	case Notification::Type_AllNodesQueried:
	case Notification::Type_AllNodesQueriedSomeDead:
		/*
		 * Allows all waiting threads to continue their execution,
		 * after driver failed.
		 */
		m_initCondition.broadcast();

		break;
	case Notification::Type_DriverReset:
	case Notification::Type_Notification:
	case Notification::Type_NodeNaming:
	case Notification::Type_NodeProtocolInfo:
	case Notification::Type_NodeQueriesComplete:
	default:
		break;
	}
}

uint32_t NotificationProcessor::homeID()
{
	return m_homeId;
}

bool NotificationProcessor::initFailed() const
{
	return m_initFailed;
}

Poco::Nullable<NodeInfo> NotificationProcessor::findNodeInfo(
	OpenZWave::Notification const *notification)
{
	return findNodeInfo(notification->GetNodeId());
}

void NotificationProcessor::setZMQClient(Poco::SharedPtr<ZMQClient> client)
{
	m_zmqClient = client;
}

void NotificationProcessor::setGenericMessageFactory(
	GenericZWaveMessageFactory *factory)
{
	m_factory = factory;
}

void onNotification(OpenZWave::Notification const *notification,
	void *context)
{
	NotificationProcessor *processor = (NotificationProcessor *) context;
	processor->onNotification(notification);
}
