#include <unistd.h>

#include <Poco/Delegate.h>

#include <Options.h>
#include <Manager.h>
#include <platform/Log.h>

#include "di/Injectable.h"
#include "z-wave/ZWaveDeviceManager.h"
#include "zmq/ZMQMessage.h"

#include "z-wave/manufacturers/AeotecZWaveMessageFactory.h"
#include "z-wave/manufacturers/DLinkZWaveMessageFactory.h"
#include "z-wave/manufacturers/FibaroZWaveMessageFactory.h"
#include "z-wave/manufacturers/PhilioZWaveMessageFactory.h"
#include "z-wave/manufacturers/PoppZWaveMessageFactory.h"

BEEEON_OBJECT_BEGIN(BeeeOn, ZWaveDeviceManager)
BEEEON_OBJECT_CASTABLE(StoppableRunnable)
BEEEON_OBJECT_TEXT("dataServerHost", &ZWaveDeviceManager::setDataServerHost)
BEEEON_OBJECT_NUMBER("dataServerPort", &ZWaveDeviceManager::setDataServerPort)
BEEEON_OBJECT_TEXT("helloServerHost", &ZWaveDeviceManager::setHelloServerHost)
BEEEON_OBJECT_NUMBER("helloServerPort", &ZWaveDeviceManager::setHelloServerPort)
BEEEON_OBJECT_TEXT("prefixName", &ZWaveDeviceManager::setPrefixName)
BEEEON_OBJECT_TEXT("setUserPath", &ZWaveDeviceManager::setUserPath)
BEEEON_OBJECT_TEXT("donglePath", &ZWaveDeviceManager::setDonglePath)
BEEEON_OBJECT_TEXT("setConfigPath", &ZWaveDeviceManager::setConfigPath)
BEEEON_OBJECT_NUMBER("setPollInterval", &ZWaveDeviceManager::setPollInterval)
BEEEON_OBJECT_NUMBER("setDriverMaxAttempts", &ZWaveDeviceManager::setDriverMaxAttempts)
BEEEON_OBJECT_NUMBER("setSaveConfigurationFile", &ZWaveDeviceManager::setSaveConfigurationFile)
BEEEON_OBJECT_END(BeeeOn, ZWaveDeviceManager)

using namespace BeeeOn;
using namespace OpenZWave;

#define QUEUE_WAIT                  50000

ZWaveDeviceManager::ZWaveDeviceManager():
	m_notificationProcessor(m_devices, m_listen),
	m_listen(false),
	m_callback(*this, &ZWaveDeviceManager::stopListen),
	m_derefListen(1000, 0),
	m_callbackUnpair(*this, &ZWaveDeviceManager::stopUnpair),
	m_derefUnpair(10000, 0)
{
	m_zmqClient->onReceive += Poco::delegate(this, &ZWaveDeviceManager::onEvent);
}

void ZWaveDeviceManager::onEvent(const void *, ZMQMessage &zmqMessage)
{
	switch(zmqMessage.type().raw()){
	case ZMQMessageType::TYPE_DEVICE_LIST_RESULT:
		doDeviceListResult(zmqMessage);
		break;
	case ZMQMessageType::TYPE_LISTEN_CMD:
		doListenCommand(zmqMessage);
		break;
	case ZMQMessageType::TYPE_DEVICE_LAST_VALUE_RESULT:
		doDeviceLastValueResult(zmqMessage);
		break;
	case ZMQMessageType::TYPE_DEVICE_UNPAIR_CMD:
		doDeviceUnpairCommand(zmqMessage);
		break;
	case ZMQMessageType::TYPE_SET_VALUES_CMD:
		doSetValueCommand(zmqMessage);
		break;
	default:
		logger().error("unsupported result " + zmqMessage.type().toString());
	}
}

void ZWaveDeviceManager::doSetValueCommand(ZMQMessage &zmqMessage)
{
	uint32_t manufacturer;
	uint32_t product;

	DeviceSetValueCommand::Ptr cmd = zmqMessage.toDeviceSetValueCommand();

	uint8_t nodeId = cmd->deviceID().ident() & 0xff;
	try {
		manufacturer = Poco::NumberParser::parseHex(
			Manager::Get()->GetNodeManufacturerId(m_homeId, nodeId));
		product = Poco::NumberParser::parseHex(
			Manager::Get()->GetNodeProductId(m_homeId, nodeId));
	}
	catch (Poco::Exception &ex) {
		logger().error("parsed failed");
		logger().log(ex, __FILE__, __LINE__);
	}

	ZWaveMessage *message = m_factory.create(manufacturer, product);

	SensorData sensorData;
	sensorData.insertValue(SensorValue(
			cmd->moduleID(),
			cmd->value()));

	message->setValue(sensorData, nodeId);
	delete message;
}

void ZWaveDeviceManager::doDeviceUnpairCommand(ZMQMessage &zmqMessage)
{
	DeviceUnpairCommand::Ptr cmd = zmqMessage.toDeviceUnpairCommand();

	m_devices.erase(cmd->deviceID());
	Manager::Get()->RemoveNode(m_homeId);

	m_derefUnpair.start(m_callbackUnpair);
}

void ZWaveDeviceManager::doDeviceLastValueResult(ZMQMessage &zmqMessage)
{
	for (auto item : m_table) {
		if (item.second.id != zmqMessage.id())
			continue;

		ServerLastValueResult::Ptr res = new ServerLastValueResult(item.first);
		zmqMessage.toServerLastValueResult(res);
	}
}

void ZWaveDeviceManager::doListenCommand(ZMQMessage &zmqMessage)
{
	GatewayListenCommand::Ptr cmd = zmqMessage.toGatewayListenCommand();
	m_derefListen.setStartInterval(cmd->duration().totalMilliseconds());
	startListen();
	Manager::Get()->AddNode(m_homeId, false);
}

void ZWaveDeviceManager::doDeviceListResult(ZMQMessage &zmqMessage)
{
	for (auto item : m_table) {
		if (item.second.id != zmqMessage.id())
			continue;

		ServerDeviceListResult::Ptr res = new ServerDeviceListResult(item.first);
		zmqMessage.toServerDeviceListResult(res);
	}
}

void ZWaveDeviceManager::setUserPath(const std::string &userPath)
{
	m_userPath = userPath;
}

void ZWaveDeviceManager::setDonglePath(const std::string &donglePath)
{
	m_donglePath = donglePath;
}

void ZWaveDeviceManager::setConfigPath(const std::string &configPath)
{
	m_configPath = configPath;
}

void ZWaveDeviceManager::setPollInterval(int pollInterval)
{
	m_pollInterval = pollInterval;
}

void ZWaveDeviceManager::setDriverMaxAttempts(int maxAttempts)
{
	m_driverMaxAttempts = maxAttempts;
}

void ZWaveDeviceManager::setSaveConfigurationFile(bool)
{
}

void ZWaveDeviceManager::installOption()
{
	OpenZWave::Options::Create(m_configPath, m_userPath, "");
	Options::Get()->AddOptionInt("PollInterval", m_pollInterval);
	Options::Get()->AddOptionBool("logging", true);
	Options::Get()->AddOptionInt("QueueLogLevel", LogLevel_Detail);
	Options::Get()->AddOptionBool("SaveConfiguration", true);
	Options::Get()->AddOptionInt("DriverMaxAttempts", m_driverMaxAttempts);
	Options::Get()->AddOptionBool("ValidateValueChanges", true);
	Options::Get()->Lock();
}

void ZWaveDeviceManager::run()
{
	installManufacturers();
	installOption();
	DeviceManager::runClient();
	sleep(1);
	m_notificationProcessor.setZMQClient(m_zmqClient);

	m_driver.assign(new ZWaveDriver(m_donglePath));
	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, &m_notificationProcessor);
	m_driver->registerItself();

	m_notificationProcessor.waitUntilQueried();
	m_homeId = m_notificationProcessor.homeID();

	getDeviceList();

	//OpenZWave::Manager::Get()->ResetController(m_homeId);
	while (!m_stop) {
		checkQueue();
	}
}

void ZWaveDeviceManager::stop()
{
	m_driver->unregisterItself();

	Manager::Get()->RemoveWatcher(onNotification, &m_notificationProcessor);
	Manager::Destroy();
	Options::Destroy();

	DeviceManager::stop();
}

void ZWaveDeviceManager::installManufacturers()
{
	m_notificationProcessor.setGenericMessageFactory(&m_factory);

	m_factory.registerManufacturer(
		AEOTEC_MANUFACTURER, new AeotecZWaveMessageFactory());
	m_factory.registerManufacturer(
		DLINK_MANUFACTURER, new DLinkZWaveMessageFactory());
	m_factory.registerManufacturer(
		FIBARO_MANUFACTURER, new FibaroZWaveMessageFactory());
	m_factory.registerManufacturer(
		PHILIO_MANUFACTURER, new PhilioZWaveMessageFactory());
	m_factory.registerManufacturer(
		POPP_MANUFACTURER, new PoppZWaveMessageFactory());
}

void ZWaveDeviceManager::getDeviceList()
{
	m_devices.clear();

	Answer::Ptr answer = new Answer(m_queue);
	ServerDeviceListCommand::Ptr cmd = new ServerDeviceListCommand(m_prefix);

	ZMQMessage msg = ZMQMessage::fromCommand(cmd);
	msg.setID(GlobalID::random());

	m_zmqClient->send(msg.toString());
	m_table.insert(make_pair(answer, ResultData{msg.id(), cmd}));

	logger().debug("run cmd: " + cmd->name());
}

void ZWaveDeviceManager::getLastValue(const DeviceID &deviceID,
	const ModuleID &moduleID)
{
	Answer::Ptr answer = new Answer(m_queue);
	ServerLastValueCommand::Ptr cmd =
		new ServerLastValueCommand(deviceID, moduleID);

	ZMQMessage msg = ZMQMessage::fromCommand(cmd);
	msg.setID(GlobalID::random());

	m_zmqClient->send(msg.toString());
	m_table.insert(make_pair(answer, ResultData{msg.id(), cmd}));

	logger().debug("run cmd: " + cmd->name());
}

void ZWaveDeviceManager::checkQueue()
{
	std::list<Answer::Ptr> dirtyList;
	m_queue.wait(QUEUE_WAIT, dirtyList);

	for (auto answer : dirtyList) {
		auto it = m_table.find(answer);

		if (it == m_table.end()) {
			logger().warning("unknown result");
			break;
		}

		if (it->second.cmd->is<ServerDeviceListCommand>()) {
			m_devices.clear();

			for (auto deviceID : answer->at(0).cast<ServerDeviceListResult>()->deviceList())
				m_devices.insert(deviceID);

			setLastState();
		}

		if (it->second.cmd->is<ServerLastValueCommand>()) {
			ServerLastValueResult::Ptr result = answer->at(0).cast<ServerLastValueResult>();
			ServerLastValueCommand::Ptr cmd = it->second.cmd.cast<ServerLastValueCommand>();

			if (result->status() != Result::SUCCESS)
				return;

			uint32_t manufacturer;
			uint32_t product;

			uint8_t nodeId = cmd->deviceID().ident() & 0xff;
			try {
				manufacturer = Poco::NumberParser::parseHex(
					OpenZWave::Manager::Get()->GetNodeManufacturerId(m_homeId, nodeId));
				product = Poco::NumberParser::parseHex(
					OpenZWave::Manager::Get()->GetNodeProductId(m_homeId, nodeId));
			}
			catch (Poco::Exception &ex) {
				logger().error("parsed failed");
				logger().log(ex, __FILE__, __LINE__);
			}

			ZWaveMessage *message = m_factory.create(manufacturer, product);

			SensorData sensorData;
			sensorData.insertValue(SensorValue(
				cmd->moduleID(),
				result->value()
			));

			message->setValue(sensorData, nodeId);
			delete message;
		}
	}
}

void ZWaveDeviceManager::startListen()
{
	logger().debug("start listen");
	m_listen = true;
	m_derefListen.start(m_callback);
}

void ZWaveDeviceManager::stopListen(Poco::Timer &)
{
	logger().debug("stop listen");
	OpenZWave::Manager::Get()->CancelControllerCommand(m_homeId);

	m_listen = false;
	getDeviceList();
}

void ZWaveDeviceManager::setLastState()
{
	for (auto item : m_devices) {
		string manufacturer =
			Manager::Get()->GetNodeManufacturerId(m_homeId, item.ident()&0xff);

		logger().debug("get last value for: " + manufacturer);
		if (Poco::NumberParser::parseHex(manufacturer) == POPP_MANUFACTURER) {
			getLastValue(item, 0);
		}

		if (Poco::NumberParser::parseHex(manufacturer) == DLINK_MANUFACTURER) {
			getLastValue(item, 4);
		}

		if (Poco::NumberParser::parseHex(manufacturer) == AEOTEC_MANUFACTURER) {
			getLastValue(item, 6);
			getLastValue(item, 8);
		}
	}
}

void ZWaveDeviceManager::stopUnpair(Poco::Timer &)
{
	logger().debug("stop unpair");
	OpenZWave::Manager::Get()->CancelControllerCommand(m_homeId);
}
