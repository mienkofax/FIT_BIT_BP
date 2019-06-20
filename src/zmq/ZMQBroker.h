#ifndef BEEEON_ZMQ_BROKER_H
#define BEEEON_ZMQ_BROKER_H

#include <map>

#include "core/AnswerQueue.h"
#include "core/CommandDispatcher.h"
#include "core/CommandHandler.h"
#include "core/Distributor.h"
#include "loop/StoppableLoop.h"
#include "model/GlobalID.h"
#include "zmq/ZMQConnector.h"
#include "zmq/ZMQDeviceManagerTable.h"
#include "zmq/FakeHandlerTest.h"

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
class ZMQBroker : public ZMQConnector, public CommandHandler {
public:
	ZMQBroker();

	bool accept(const Command::Ptr cmd) override;
	void handle(Command::Ptr cmd, Answer::Ptr answer) override;

	void run() override;

	void setDistributor(Poco::SharedPtr<Distributor> distributor);

	void setCommandDispatcher(Poco::SharedPtr<CommandDispatcher> dispatcher);

	unsigned long deviceManagersCount();

	void setFakeHandlerTest(Poco::SharedPtr<FakeHandlerTest> handler);

protected:
	void configureDataSockets() override;
	void configureHelloSockets() override;

	void dataServerReceive() override;
	void helloServerReceive() override;

	void handleHelloMessage(ZMQMessage &zmqMessage);
	void handleDataMessage(ZMQMessage &zmqMessage,
		const DeviceManagerID &deviceManagerID);

	/*
	 * Registruje nove zariadenie do zoznamu pripojenych device
	 * mangerov a novo pripojenemu device manageru priradi ID.
	 */
	void registerDeviceManager(ZMQMessage &zmqMessage);

	void checkQueue();

	void checkSettingTable(Poco::Timer &timer);

	void doDefaultResult(ZMQMessage &zmqMessage);

	void doDeviceLastValueCommand(ZMQMessage &zmqMessage,
		const DeviceManagerID &deviceManagerID);

	void doDeviceListCommand(ZMQMessage &zmqMessage,
		const DeviceManagerID &deviceManagerID);

protected:
	struct ResultData {
		GlobalID resultID;
		DeviceManagerID deviceManagerID;
		Command::Ptr cmd;
	};

	struct ResultData2 {
		Answer::Ptr answer;
		Command::Ptr cmd;
		Result::Ptr result;
	};

protected:
	Poco::SharedPtr<Distributor> m_distributor;
	Poco::SharedPtr<CommandDispatcher> m_commandDispatcher;
	ZMQDeviceManagerTable m_deviceManagersTable;
	std::map<Answer::Ptr, ResultData> m_resultTable;
	AnswerQueue m_answerQueue;

	std::map<GlobalID, ResultData2> m_cmdTable;
	Poco::SharedPtr<FakeHandlerTest> m_fakeHandlerTest;

	Poco::Timer m_timer;
	Poco::TimerCallback<ZMQBroker> m_callback;

	struct SettingValueItem {
		Command::Ptr cmd;
		Answer::Ptr answer;
		Poco::Timestamp endTime;
	};
	std::map<DeviceID, SettingValueItem> m_settingTable;
};

}

#endif
