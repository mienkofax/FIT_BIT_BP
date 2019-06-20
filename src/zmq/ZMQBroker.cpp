#include <unistd.h>

#include "di/Injectable.h"
#include "model/DeviceManagerID.h"
#include "model/SensorData.h"
#include "util/ZMQUtil.h"
#include "zmq/ZMQBroker.h"
#include "zmq/ZMQMessage.h"

BEEEON_OBJECT_BEGIN(BeeeOn, ZMQBroker)
BEEEON_OBJECT_CASTABLE(StoppableRunnable)
BEEEON_OBJECT_CASTABLE(CommandHandler)
BEEEON_OBJECT_TEXT("dataServerHost", &ZMQBroker::setDataServerHost)
BEEEON_OBJECT_NUMBER("dataServerPort", &ZMQBroker::setDataServerPort)
BEEEON_OBJECT_TEXT("helloServerHost", &ZMQBroker::setHelloServerHost)
BEEEON_OBJECT_NUMBER("helloServerPort", &ZMQBroker::setHelloServerPort)
BEEEON_OBJECT_REF("distributor", &ZMQBroker::setDistributor)
BEEEON_OBJECT_REF("commandDispatcher", &ZMQBroker::setCommandDispatcher)
BEEEON_OBJECT_REF("fakeHandlerTest", &ZMQBroker::setFakeHandlerTest)
BEEEON_OBJECT_END(BeeeOn, ZMQBroker)

const int LOOP_USLEEP = 100;
const int QUEUE_WAIT = 10000;

using namespace BeeeOn;
using namespace Poco;
using namespace std;

ZMQBroker::ZMQBroker():
	ZMQConnector(),
	CommandHandler("ZMQBroker"),
	m_timer(0, 1000),
	m_callback(*this, &ZMQBroker::checkSettingTable)
{
	m_timer.start(m_callback);
}

void ZMQBroker::setDistributor(SharedPtr<Distributor> distributor)
{
	m_distributor = distributor;
}

bool ZMQBroker::accept(const Command::Ptr cmd)
{
	if (cmd->is<DeviceSetValueCommand>()) {
		return m_deviceManagersTable.isDeviceManagerRegistered(
			cmd.cast<DeviceSetValueCommand>()->deviceID().prefix());
	}
	else if (cmd->is<DeviceUnpairCommand>()) {
		m_deviceManagersTable.isDeviceManagerRegistered(
			cmd.cast<DeviceUnpairCommand>()->deviceID().prefix());
		return true;
	}
	else if (cmd->is<GatewayListenCommand>()) {
		return true;
	}

	return false;
}

void ZMQBroker::handle(Command::Ptr cmd, Answer::Ptr answer)
{
	vector<DeviceManagerID> managers;

	if (cmd->is<DeviceSetValueCommand>()) {
		DeviceSetValueCommand::Ptr setCmd = cmd.cast<DeviceSetValueCommand>();
		managers = m_deviceManagersTable.getAll(setCmd->deviceID().prefix());

		Timestamp endTime = Timestamp() + setCmd->timeout().totalMicroseconds();
		m_settingTable.insert(make_pair(
				setCmd->deviceID(),
				SettingValueItem{cmd, answer, endTime}));
	}
	else if (cmd->is<DeviceUnpairCommand>()) {
		managers = m_deviceManagersTable.getAll(
			cmd.cast<DeviceUnpairCommand>()->deviceID().prefix());
	}
	else if (cmd->is<GatewayListenCommand>()) {
		managers = m_deviceManagersTable.getAll();
	}

	for (auto deviceManagerID : managers) {
		ZMQMessage msg = ZMQMessage::fromCommand(cmd);

		m_cmdTable.insert(
			make_pair(msg.id(), ResultData2{answer, cmd, new Result(answer)}));

		ZMQUtil::sendMultipart(m_dataServerSocket, deviceManagerID.toString());
		ZMQUtil::send(m_dataServerSocket, msg.toString());
	}
}

void ZMQBroker::run()
{
	configureDataSockets();
	configureHelloSockets();

	while(!m_stop) {
		dataServerReceive();
		helloServerReceive();
		checkQueue();
		usleep(LOOP_USLEEP);
	}

	if (logger().debug())
		logger().debug("ZMQ_REP and ZMQ_ROUTER stop");

	m_timer.stop();
}

void ZMQBroker::checkQueue()
{
	std::list<Answer::Ptr> dirtyList;
	m_answerQueue.wait(QUEUE_WAIT, dirtyList);

	for (auto &answer : dirtyList) {
		for (unsigned long i = 0; i < answer->resultsCount(); ++i) {
			auto it = m_resultTable.find(answer);

			ZMQUtil::sendMultipart(m_dataServerSocket,
				it->second.deviceManagerID.toString());

			ZMQMessage msg = ZMQMessage::fromResult(answer->at(i));
			msg.setID(it->second.resultID);
			ZMQUtil::send(m_dataServerSocket, msg.toString());
		}
	}
}

void ZMQBroker::configureDataSockets()
{
	m_dataServerSocket.assign(new zmq::socket_t(m_context, ZMQ_ROUTER));

	string address = createAddress(m_dataServerHost, m_dataServerPort);

	try {
		m_dataServerSocket->bind(address);

		if (logger().debug())
			logger().debug("zmq data server is running on: " + address);
	}
	catch (zmq::error_t &ex) {
		logger().warning(string(ex.what()) + ": " + address);
		stop();
	}
	catch (InvalidArgumentException &ex) {
		logger().error("wrong type of socket address: " + address);
		logger().log(ex, __FILE__, __LINE__);
		stop();
	}
}

void ZMQBroker::configureHelloSockets()
{
	m_helloServerSocket.assign(new zmq::socket_t(m_context, ZMQ_REP));

	string address = createAddress(m_helloServerHost, m_helloServerPort);

	try {
		m_helloServerSocket->bind(address);

		if (logger().debug())
			logger().debug("zmq hello server is running on: " + address);
	}
	catch(zmq::error_t &ex) {
		logger().warning(string(ex.what()) + ": " + address);
		stop();
	}
	catch (InvalidArgumentException &ex) {
		logger().error("wrong type of socket address: " + address);
		logger().log(ex, __FILE__, __LINE__);
		stop();
	}
}

void ZMQBroker::helloServerReceive()
{
	string jsonMessage;

	if (!ZMQUtil::receive(m_helloServerSocket, jsonMessage))
		return;

	if (logger().debug())
		logger().debug("broker receive data (helloServerSocket):\n"
			+ jsonMessage);

	ZMQMessage zmqMessage;
	if (!parseMessage(jsonMessage, m_dataServerSocket, zmqMessage))
		return;

	try {
		handleHelloMessage(zmqMessage);
	}
	catch (InvalidAccessException &ex) {
		logger().warning("missing JSON attribute");
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MISSING_ATTRIBUTE,
			"missing JSON attribute",
			m_dataServerSocket);
	}
}

void ZMQBroker::handleHelloMessage(ZMQMessage &zmqMessage)
{
	switch (zmqMessage.type().raw()) {
	case ZMQMessageType::TYPE_HELLO_REQUEST:
		registerDeviceManager(zmqMessage);
		break;
	default:
		sendError(
			ZMQMessageError::ERROR_UNSUPPORTED_MESSAGE,
			"unsupported message type",
			m_dataServerSocket);
	}
}

void ZMQBroker::dataServerReceive()
{
	string deviceManagerID;
	string jsonMessage;

	if (!ZMQUtil::receive(m_dataServerSocket, deviceManagerID)
		|| !ZMQUtil::receive(m_dataServerSocket, jsonMessage))
		return;

	if (logger().debug()) {
		logger().debug(
			"broker receive data (dataServerSocket) from: "
			+ deviceManagerID + "\n"
			+ jsonMessage);
	}

	ZMQMessage zmqMessage;
	if (!parseMessage(jsonMessage, m_dataServerSocket, zmqMessage))
		return;

	try {
		handleDataMessage(zmqMessage, DeviceManagerID::parse(deviceManagerID));
	}
	catch (const InvalidAccessException &ex) {
		logger().warning("missing JSON attribute");
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MISSING_ATTRIBUTE,
			"missing JSON attribute",
			m_dataServerSocket);
	}
	catch (const Exception &ex) {
		logger().log(ex, __FILE__, __LINE__);
	}
}

void ZMQBroker::handleDataMessage(ZMQMessage &zmqMessage,
		const DeviceManagerID &deviceManagerID)
{
	switch (zmqMessage.type().raw()) {
	case ZMQMessageType::TYPE_MEASURED_VALUES:
		m_distributor->exportData(zmqMessage.toSensorData());
		m_fakeHandlerTest->addPairedDeviceID(zmqMessage.toSensorData().deviceID()); // for testing
		break;
	case ZMQMessageType::TYPE_DEFAULT_RESULT:
		doDefaultResult(zmqMessage);
		break;
	case ZMQMessageType::TYPE_DEVICE_LAST_VALUE_CMD:
		doDeviceLastValueCommand(zmqMessage, deviceManagerID);
		break;
	case ZMQMessageType::TYPE_DEVICE_LIST_CMD:
		doDeviceListCommand(zmqMessage, deviceManagerID);
		break;
	default:
		sendError(
			ZMQMessageError::ERROR_UNSUPPORTED_MESSAGE,
			"unsupported message type",
			m_dataServerSocket);
	}
}

void ZMQBroker::doDeviceListCommand(ZMQMessage &zmqMessage,
	const DeviceManagerID &deviceManagerID)
{
	Answer::Ptr answer = new Answer(m_answerQueue);
	ServerDeviceListCommand::Ptr cmd = zmqMessage.toDeviceListRequest();

	m_resultTable.insert(
		make_pair(answer, ResultData{zmqMessage.id(),deviceManagerID, cmd}));

	m_commandDispatcher->dispatch(cmd, answer);
}

void ZMQBroker::doDeviceLastValueCommand(ZMQMessage &zmqMessage,
	const DeviceManagerID &deviceManagerID)
{
	Answer::Ptr answer =  new Answer(m_answerQueue);
	ServerLastValueCommand::Ptr cmd = zmqMessage.toServerLastValueCommand();

	m_resultTable.insert(
		make_pair(answer, ResultData{zmqMessage.id(),deviceManagerID, cmd}));

	m_commandDispatcher->dispatch(cmd, answer);
}

void ZMQBroker::doDefaultResult(ZMQMessage &zmqMessage)
{
	auto it = m_cmdTable.find(zmqMessage.id());

	if (it == m_cmdTable.end())
		logger().warning("unknown result id");

	Result::Ptr result = it->second.result;

	zmqMessage.toDefaultResult(result);
}

void ZMQBroker::setCommandDispatcher(SharedPtr<CommandDispatcher> dispatcher)
{
	m_commandDispatcher = dispatcher;
}

unsigned long ZMQBroker::deviceManagersCount()
{
	return m_deviceManagersTable.count();
}

void ZMQBroker::registerDeviceManager(ZMQMessage &zmqMessage)
{
	try {
		DeviceManagerID deviceManagerID =
			m_deviceManagersTable.registerDaemonPrefix(
				zmqMessage.toHelloRequest());

		if (logger().debug())
			logger().debug("register device manager id: "
				+ deviceManagerID.toString());

		ZMQMessage msg = ZMQMessage::fromHelloResponse(deviceManagerID);

		ZMQUtil::send(m_helloServerSocket, msg.toString());
	}
	catch(RangeException &ex) {
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MAXIMUM_DEVICE_MANAGERS,
			"maximum number of registered device managers",
			m_helloServerSocket);
	}
}

void ZMQBroker::setFakeHandlerTest(SharedPtr<FakeHandlerTest> handler)
{
	m_fakeHandlerTest = handler;
}

void ZMQBroker::checkSettingTable(Timer &)
{
	for (auto item : m_settingTable) {
		if (item.second.answer->resultsCount() == 0)
			continue;

		DeviceSetValueResult::Ptr result =
			item.second.answer->at(0).cast<DeviceSetValueResult>();

		if (Timestamp() > item.second.endTime) {
			result->setExtendetSetStatus(DeviceSetValueResult::GW_DEVICE_TIMEOUT);
			m_settingTable.erase(item.first);
		}

		if (result->extendetSetStatus() == DeviceSetValueResult::DEVICE_FAILED) {
			result->setExtendetSetStatus(DeviceSetValueResult::GW_DEVICE_FAILED);
			m_settingTable.erase(item.first);
		}

		if (result->extendetSetStatus() == DeviceSetValueResult::DEVICE_SUCCESS) {
			result->setExtendetSetStatus(DeviceSetValueResult::GW_DEVICE_SUCCESS);
			m_settingTable.erase(item.first);
		}
	}

}
