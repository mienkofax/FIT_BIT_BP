#include "ZMQUtil.h"

using namespace BeeeOn;
using namespace std;
using namespace Poco;

int ZMQUtil::receive(SharedPtr<zmq::socket_t> socket,
	string &message)
{
	zmq::message_t zmqMessage;
	int returnCode = socket->recv(&zmqMessage, ZMQ_DONTWAIT);

	if (returnCode == -1)
		return returnCode;

	message = string(static_cast<char*>(zmqMessage.data()), zmqMessage.size());

	return returnCode;
}

bool ZMQUtil::send(SharedPtr<zmq::socket_t> socket, const string &message)
{
	zmq::message_t zmqMessage(message.size());
	memcpy(zmqMessage.data(), message.data(), message.size());

	return socket->send(zmqMessage);
}

bool ZMQUtil::sendMultipart(SharedPtr<zmq::socket_t> socket,
	const string &message)
{
	zmq::message_t zmqMessage(message.size());
	memcpy(zmqMessage.data(), message.data(), message.size());

	return socket->send(zmqMessage, ZMQ_SNDMORE);
}
