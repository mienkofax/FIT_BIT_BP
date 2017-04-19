#include <Poco/Net/SocketAddress.h>

#include "util/ZMQUtil.h"
#include "zmq/ZMQConnector.h"
#include "zmq/ZMQMessage.h"

using namespace BeeeOn;
using namespace Poco;
using namespace std;

const int DEFAULT_IO_THREAD = 1;

ZMQConnector::ZMQConnector():
	m_stop(0),
	m_context(DEFAULT_IO_THREAD)
{
}

ZMQConnector::~ZMQConnector()
{
	if (m_dataServerSocket)
		m_dataServerSocket->close();

	if (m_helloServerSocket)
		m_helloServerSocket->close();
}

void ZMQConnector::stop()
{
	m_stop = 1;
}

void ZMQConnector::setDataServerHost(const string &host)
{
	m_dataServerHost = host;
}

void ZMQConnector::setHelloServerHost(const string &host)
{
	m_helloServerHost = host;
}

void ZMQConnector::setDataServerPort(const int port)
{
	m_dataServerPort = port;
}

void ZMQConnector::setHelloServerPort(const int port)
{
	m_helloServerPort = port;
}

string ZMQConnector::createAddress(const string &host, int port)
{
	Net::SocketAddress socketAddress;

	try {
		socketAddress = Net::SocketAddress(host, port);
	}
	catch (Poco::Exception &ex) {
		throw;
	}

	return "tcp://" + socketAddress.toString();
}

bool ZMQConnector::parseMessage(const std::string &jsonMessage,
	Poco::SharedPtr<zmq::socket_t> socket, ZMQMessage &msg)
{
	try {
		msg = ZMQMessage::fromJSON(jsonMessage);

		msg.type();
	}
	catch (Poco::InvalidAccessException &ex) {
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MISSING_ATTRIBUTE,
			"missing attribute: message_type",
			socket);

		return false;

	}
	catch (Poco::InvalidArgumentException &ex) {
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_MISSING_ATTRIBUTE,
			"unknown message_type",
			socket);

		return false;
	}
	catch (Poco::SyntaxException &ex) {
		logger().log(ex, __FILE__, __LINE__);

		sendError(
			ZMQMessageError::ERROR_JSON_SYNTAX,
			"json syntax",
			socket);

		return false;
	}

	return true;
}

int ZMQConnector::sendError(const ZMQMessageError::Error errorType,
	const std::string message, Poco::SharedPtr<zmq::socket_t> socket)
{
	ZMQMessage errorMessage =
		ZMQMessage::fromError(errorType, message);

	return ZMQUtil::send(socket, errorMessage.toString());
}
