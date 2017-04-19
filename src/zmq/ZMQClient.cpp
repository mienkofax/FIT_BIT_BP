#include <unistd.h>

#include "di/Injectable.h"
#include "util/ZMQUtil.h"
#include "zmq/ZMQClient.h"
#include "zmq/ZMQMessage.h"

const int LOOP_USLEEP = 100;

using namespace BeeeOn;
using namespace std;

ZMQClient::ZMQClient():
	ZMQConnector(),
	m_devicePrefix(DevicePrefix::parse("Invalid"))
{
}

void ZMQClient::setDeviceManagerPrefix(const DevicePrefix &prefix)
{
	m_devicePrefix = prefix;
}

Poco::Nullable<DeviceManagerID> ZMQClient::deviceManagerID()
{
	return m_deviceMangerID;
}

void ZMQClient::run()
{
	configureHelloSockets();

	ZMQMessage helloRequest = ZMQMessage::fromHelloRequest(m_devicePrefix);
	ZMQUtil::send(m_helloServerSocket, helloRequest.toString());

	while (!m_stop) {
		if (!m_deviceMangerID.isNull()) {
			configureDataSockets();
			break;
		}

		helloServerReceive();
		usleep(LOOP_USLEEP);
	}

	while (!m_stop) {
		dataServerReceive();
		usleep(LOOP_USLEEP);
	}

	if (logger().debug())
		logger().debug("ZMQ_REP and ZMQ_ROUTER stop");
}

void ZMQClient::configureDataSockets()
{
	m_dataServerSocket.assign(new zmq::socket_t(m_context, ZMQ_DEALER));

	string address = createAddress(m_dataServerHost, m_dataServerPort);

	try {
		string identity = m_deviceMangerID.value().toString();
		m_dataServerSocket->setsockopt(
			ZMQ_IDENTITY, identity.c_str(), identity.size());

		m_dataServerSocket->connect(address);

		if (logger().debug())
			logger().debug("zmq data client is running on: " + address);
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

void ZMQClient::configureHelloSockets()
{
	m_helloServerSocket.assign(new zmq::socket_t(m_context, ZMQ_REQ));

	string address = createAddress(m_helloServerHost, m_helloServerPort);

	try {
		m_helloServerSocket->connect(address);

		if (logger().debug())
			logger().debug("zmq hello client is running on: " + address);
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

void ZMQClient::helloServerReceive()
{
	string jsonMessage;

	if (!ZMQUtil::receive(m_helloServerSocket, jsonMessage))
		return;

	if (logger().debug())
		logger().debug("client receive data (helloServerSocket):\n"
			+ jsonMessage);

	ZMQMessage zmqMessage;
	if (!parseMessage(jsonMessage, m_dataServerSocket, zmqMessage))
		return;

	switch (zmqMessage.type().raw()) {
	case ZMQMessageType::TYPE_HELLO_RESPONSE:
		m_deviceMangerID = zmqMessage.toHelloResponse();

		if (logger().debug()) {
			logger().debug("assigned device manger ID: "
				+ m_deviceMangerID.value().toString());
		}
		break;
	default:
		sendError(
			ZMQMessageError::ERROR_UNSUPPORTED_MESSAGE,
			"unsupported message type",
			m_dataServerSocket);
	}
}

void ZMQClient::dataServerReceive()
{
	string deviceManagerID;
	string jsonMessage;

	if (!ZMQUtil::receive(m_dataServerSocket, jsonMessage))
		return;

	if (logger().debug())
		logger().debug("client receive data (dataServerSocket):\n"
			+ jsonMessage);

	ZMQMessage zmqMessage;
	if (!parseMessage(jsonMessage, m_dataServerSocket, zmqMessage))
		return;

	onReceive(this, zmqMessage);
}

int ZMQClient::send(const std::string &message)
{
	return ZMQUtil::send(m_dataServerSocket, message);
}
