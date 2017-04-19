#ifndef BEEEON_ZMQ_BROKER_H
#define BEEEON_ZMQ_BROKER_H

#include "core/CommandDispatcher.h"
#include "core/Distributor.h"
#include "zmq/ZMQConnector.h"

namespace BeeeOn {

class ZMQProtocolMessage;

class DeviceManagerID;
class Distributor;

/*
 * ZMQBroker ensures data receiving using zmq protocol.
 * It contains two sockets: dataSocket and helloSocket.
 * The DataSocket enables to send and receive messages
 * asynchronously and it serves for sending of commands
 * and measured values. The type of the HelloSocket is a
 * server-client and this socket serves for registering of
 * device mangers to enable communication using dataSocket.
 */
class ZMQBroker : public ZMQConnector {
public:
	ZMQBroker();

	void run() override;

	void setDistributor(Poco::SharedPtr<Distributor> distributor);

	void setCommandDispatcher(Poco::SharedPtr<CommandDispatcher> dispatcher);

	unsigned long deviceManagersCount();

protected:
	void configureDataSockets() override;
	void configureHelloSockets() override;

	void dataServerReceive() override;
	void helloServerReceive() override;

	void handleHelloMessage(ZMQMessage &zmqMessage);
	void handleDataMessage(ZMQMessage &zmqMessage,
		const DeviceManagerID &deviceManagerID);

protected:
	Poco::SharedPtr<Distributor> m_distributor;
	Poco::SharedPtr<CommandDispatcher> m_commandDispatcher;
};

}

#endif
