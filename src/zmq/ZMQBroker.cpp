#include <unistd.h>

#include "di/Injectable.h"
#include "model/DeviceManagerID.h"
#include "util/ZMQUtil.h"
#include "zmq/ZMQBroker.h"
#include "zmq/ZMQMessage.h"

BEEEON_OBJECT_BEGIN(BeeeOn, ZMQBroker)
BEEEON_OBJECT_CASTABLE(StoppableRunnable)
BEEEON_OBJECT_TEXT("dataServerHost", &ZMQBroker::setDataServerHost)
BEEEON_OBJECT_NUMBER("dataServerPort", &ZMQBroker::setDataServerPort)
BEEEON_OBJECT_TEXT("helloServerHost", &ZMQBroker::setHelloServerHost)
BEEEON_OBJECT_NUMBER("helloServerPort", &ZMQBroker::setHelloServerPort)
BEEEON_OBJECT_REF("distributor", &ZMQBroker::setDistributor)
BEEEON_OBJECT_REF("commandDispatcher", &ZMQBroker::setCommandDispatcher)
BEEEON_OBJECT_END(BeeeOn, ZMQBroker)

const int LOOP_USLEEP = 100;

using namespace BeeeOn;
using namespace std;

ZMQBroker::ZMQBroker():
	ZMQConnector()
{
}

void ZMQBroker::setDistributor(Poco::SharedPtr<Distributor> distributor)
{
	m_distributor = distributor;
}

void ZMQBroker::run()
{
	configureDataSockets();
	configureHelloSockets();

	while(!m_stop) {
		dataServerReceive();
		helloServerReceive();
		usleep(LOOP_USLEEP);
	}

	if (logger().debug())
		logger().debug("ZMQ_REP and ZMQ_ROUTER stop");
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
	catch (Poco::InvalidArgumentException &ex) {
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
	catch (Poco::InvalidArgumentException &ex) {
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
	catch (Poco::InvalidAccessException &ex) {
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
	catch (Poco::InvalidAccessException &ex) {
		logger().warning("missing JSON attribute");
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MISSING_ATTRIBUTE,
			"missing JSON attribute",
			m_dataServerSocket);
	}
	catch (Poco::Exception &ex) {
		logger().log(ex, __FILE__, __LINE__);
	}
}

void ZMQBroker::handleDataMessage(ZMQMessage &zmqMessage,
		const DeviceManagerID &deviceManagerID)
{
	switch (zmqMessage.type().raw()) {
	default:
		sendError(
			ZMQMessageError::ERROR_UNSUPPORTED_MESSAGE,
			"unsupported message type",
			m_dataServerSocket);
	}
}

void ZMQBroker::setCommandDispatcher(Poco::SharedPtr<CommandDispatcher> dispatcher)
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
	catch(Poco::RangeException &ex) {
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MAXIMUM_DEVICE_MANAGERS,
			"maximum number of registered device managers",
			m_helloServerSocket);
	}
}
