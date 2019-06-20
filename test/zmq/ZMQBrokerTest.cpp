#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>

#include <Poco/Delegate.h>
#include <Poco/Random.h>

#include "core/BasicDistributor.h"
#include "loop/LoopRunner.h"
#include "util/ZMQUtil.h"
#include "zmq/ZMQBroker.h"
#include "zmq/ZMQClient.h"
#include "zmq/ZMQMessage.h"

namespace BeeeOn {

class ZMQBrokerTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ZMQBrokerTest);
	CPPUNIT_TEST(testAssignDeviceManagerID);
	CPPUNIT_TEST(testListenCommandOneClient);
	CPPUNIT_TEST(testListenCommandTwoClients);
	CPPUNIT_TEST(testUnpairCommand);
	CPPUNIT_TEST_SUITE_END();

public:
	void testAssignDeviceManagerID();
	void testListenCommandOneClient();
	void testListenCommandTwoClients();
	void testUnpairCommand();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ZMQBrokerTest);

class FakeClient : public ZMQClient {
public:
	FakeClient():
		ZMQClient()
	{
		onReceive += Poco::delegate(this, &FakeClient::handleMessage);
	}

	void handleMessage(const void*, ZMQMessage& zmqMessage)
	{
		m_zmqMessage = zmqMessage;
		m_event.set();

		switch (zmqMessage.type().raw()){
		case ZMQMessageType::TYPE_LISTEN_CMD:
		case ZMQMessageType::TYPE_DEVICE_UNPAIR_CMD:
		case ZMQMessageType::TYPE_SET_VALUES_CMD: {
			AnswerQueue queue;
			Answer::Ptr answer = new Answer(queue);
			Result::Ptr result = new Result(answer);
			result->setStatus(Result::SUCCESS);

			ZMQMessage msgResult = ZMQMessage::fromResult(result);
			msgResult.setID(zmqMessage.id());
			send(msgResult.toString());

			break;
		}
		default:
			throw Poco::InvalidArgumentException("unsupported msg");
		}
	}

	bool waitOnMessage(Poco::Timespan timeout, ZMQMessage &msg){
		bool wait = m_event.tryWait(timeout.totalMilliseconds());

		msg = m_zmqMessage;
		return wait;
	}
private:
	Poco::Event m_event;
	ZMQMessage m_zmqMessage;
};

class FakeBroker : public ZMQBroker {
public:
	FakeBroker():
		ZMQBroker(),
		m_clients(INT_MAX)
	{
	}

	virtual ~FakeBroker(){}

	/*
	 * Posle danu spravu vsetkym klientom po tom co sa dosiahne
	 * zadany pocet klientov.
	 */
	void sendMessage(const ZMQMessage &zmqMessage,
		unsigned long clients = 1)
	{
		m_zmqMessage = zmqMessage;
		m_clients = clients;
	}

	void run() override
	{
		configureDataSockets();
		configureHelloSockets();

		while(!m_stop) {
			if (deviceManagersCount() == m_clients) {
				for (auto id : m_deviceManagersTable.getAll()) {
					ZMQUtil::sendMultipart(m_dataServerSocket, id.toString());
					ZMQUtil::send(m_dataServerSocket, m_zmqMessage.toString());
				}
				break;
			}

			dataServerReceive();
			helloServerReceive();
			checkQueue();
			usleep(100);
		}

		if (logger().debug())
			logger().debug("ZMQ_REP and ZMQ_ROUTER stop");
	}

private:
	ZMQMessage m_zmqMessage;
	unsigned long m_clients;
};

class InitComponents {
public:
	InitComponents():
		m_distributor(new BasicDistributor()),
		m_commandDispatcher(new CommandDispatcher())
	{
		Poco::Random random;

		m_helloPort = 10000 + random.next(10000);
		m_dataPort = 10000 + random.next(10000);
	}

	Poco::SharedPtr<FakeBroker> addServer()
	{
		m_broker.assign(new FakeBroker());

		m_broker->setDataServerHost("127.0.0.1");
		m_broker->setDataServerPort(m_dataPort);
		m_broker->setHelloServerHost("127.0.0.1");
		m_broker->setHelloServerPort(m_helloPort);

		m_broker->setDistributor(m_distributor);
		m_broker->setCommandDispatcher(m_commandDispatcher);
		m_commandDispatcher->registerHandler(m_broker);

		m_runner.addRunnable(m_broker);

		return m_broker;
	}

	Poco::SharedPtr<FakeClient> addClient(const DevicePrefix &prefix)
	{
		Poco::SharedPtr<FakeClient> client(new FakeClient());

		client->setDataServerHost("127.0.0.1");
		client->setDataServerPort(m_dataPort);
		client->setHelloServerHost("127.0.0.1");
		client->setHelloServerPort(m_helloPort);
		client->setDeviceManagerPrefix(prefix);

		m_runner.addRunnable(client);
		m_clients.push_back(client);

		return client;
	}

	Poco::SharedPtr<CommandDispatcher> commandDispatcher()
	{
		return m_commandDispatcher;
	}

	void start()
	{
		m_runner.start();
	}

	void stop()
	{
		m_runner.stop();
	}

private:
	LoopRunner m_runner;
	Poco::SharedPtr<FakeBroker> m_broker;
	std::vector<Poco::SharedPtr<FakeClient>> m_clients;
	Poco::SharedPtr<Distributor> m_distributor;
	Poco::SharedPtr<CommandDispatcher> m_commandDispatcher;

	int m_helloPort;
	int m_dataPort;
};

/*
 * Overenie, ze po starte clienta sa mu nastavi spravne ID na zaklade
 * device prefixu.
 *
 * Overenie ze odoslana listen sprava prisla bez chyby a moze sa dalej
 * spracovat.
 */
void ZMQBrokerTest::testAssignDeviceManagerID()
{
	InitComponents init;

	Poco::SharedPtr<FakeBroker> broker = init.addServer();
	Poco::SharedPtr<FakeClient> client1 = init.addClient(DevicePrefix::parse("Z-Wave"));
	Poco::SharedPtr<FakeClient> client2 = init.addClient(DevicePrefix::parse("Jablotron"));

	init.start();

	// cakanie maximalne 2s pokial sa nastavia ID
	for (int i = 0; i < 200; i++) {
		if (broker->deviceManagersCount() == 2) {

			break;
		}

		usleep(10000);
	}
	usleep(100000);

	CPPUNIT_ASSERT(!client1->deviceManagerID().isNull());
	DeviceManagerID id1 = client1->deviceManagerID().value();
	CPPUNIT_ASSERT(id1.prefix() == DevicePrefix::parse("Z-Wave"));

	CPPUNIT_ASSERT(!client2->deviceManagerID().isNull());
	DeviceManagerID id2 = client2->deviceManagerID().value();
	CPPUNIT_ASSERT(id2.prefix() == DevicePrefix::parse("Jablotron"));

	ZMQMessage sendMsg = ZMQMessage::fromCommand(new GatewayListenCommand(10000));
	sendMsg.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));
	broker->sendMessage(sendMsg, 2);

	ZMQMessage receiveMsg;
	CPPUNIT_ASSERT(client1->waitOnMessage(3000000, receiveMsg));
	CPPUNIT_ASSERT(sendMsg.toString() == receiveMsg.toString());

	init.stop();
	sleep(1);
}

void ZMQBrokerTest::testListenCommandOneClient()
{
	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	InitComponents init;
	std::list<Answer::Ptr> dirtyList;

	Poco::SharedPtr<FakeBroker> broker = init.addServer();
	Poco::SharedPtr<FakeClient> client1 = init.addClient(DevicePrefix::parse("Z-Wave"));
	CommandDispatcher *commandDispatcher = init.commandDispatcher();

	init.start();

	// cakanie maximalne 2s pokial sa nastavia ID
	for (int i = 0; i < 200; i++) {
		if (broker->deviceManagersCount() == 1) {

			break;
		}

		usleep(10000);
	}
	usleep(100000);
	CPPUNIT_ASSERT(1 == broker->deviceManagersCount());

	GatewayListenCommand::Ptr cmd =
		new GatewayListenCommand(60*Poco::Timespan::SECONDS);

	commandDispatcher->dispatch(cmd, answer);

	ZMQMessage receiveMessage;
	client1->waitOnMessage(600000, receiveMessage);
	CPPUNIT_ASSERT(receiveMessage.type() == ZMQMessageType::TYPE_LISTEN_CMD);

	queue.wait(1000000, dirtyList);
	CPPUNIT_ASSERT(1 == dirtyList.size());
	CPPUNIT_ASSERT(answer->at(0)->status() == Result::SUCCESS);

	init.stop();
	sleep(1);
}

void ZMQBrokerTest::testListenCommandTwoClients()
{
	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	InitComponents init;
	std::list<Answer::Ptr> dirtyList;

	Poco::SharedPtr<FakeBroker> broker = init.addServer();
	Poco::SharedPtr<FakeClient> client1 = init.addClient(DevicePrefix::parse("Z-Wave"));
	Poco::SharedPtr<FakeClient> client2 = init.addClient(DevicePrefix::parse("Z-Wave"));
	CommandDispatcher *commandDispatcher = init.commandDispatcher();

	init.start();

	// cakanie maximalne 2s pokial sa nastavia ID
	for (int i = 0; i < 200; i++) {
		if (broker->deviceManagersCount() == 2) {

			break;
		}

		usleep(10000);
	}
	usleep(100000);
	CPPUNIT_ASSERT(2 == broker->deviceManagersCount());

	GatewayListenCommand::Ptr cmd =
		new GatewayListenCommand(60*Poco::Timespan::SECONDS);

	commandDispatcher->dispatch(cmd, answer);

	ZMQMessage receiveMessage;
	client1->waitOnMessage(600000, receiveMessage);
	CPPUNIT_ASSERT(receiveMessage.type() == ZMQMessageType::TYPE_LISTEN_CMD);

	CPPUNIT_ASSERT(queue.wait(1000000, dirtyList));
	CPPUNIT_ASSERT(1 == dirtyList.size());

	ZMQMessage receiveMessage2;
	client2->waitOnMessage(600000, receiveMessage2);
	CPPUNIT_ASSERT(receiveMessage2.type() == ZMQMessageType::TYPE_LISTEN_CMD);

	dirtyList.clear();
	CPPUNIT_ASSERT(queue.wait(1000000, dirtyList));
	CPPUNIT_ASSERT(1 == dirtyList.size());

	CPPUNIT_ASSERT(answer->at(0)->status() == Result::SUCCESS);
	CPPUNIT_ASSERT(answer->at(1)->status() == Result::SUCCESS);

	init.stop();
	sleep(1);
}

void ZMQBrokerTest::testUnpairCommand()
{
	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	InitComponents init;
	std::list<Answer::Ptr> dirtyList;

	Poco::SharedPtr<FakeBroker> broker = init.addServer();
	Poco::SharedPtr<FakeClient> client1 = init.addClient(DevicePrefix::parse("Z-Wave"));
	CommandDispatcher *commandDispatcher = init.commandDispatcher();

	init.start();

	// cakanie maximalne 2s pokial sa nastavia ID
	for (int i = 0; i < 200; i++) {
		if (broker->deviceManagersCount() == 1) {

			break;
		}

		usleep(10000);
	}
	usleep(100000);
	CPPUNIT_ASSERT(1 == broker->deviceManagersCount());

	DeviceSetValueCommand::Ptr cmd = new DeviceSetValueCommand(
		DeviceID(0xa801020304050607),
		ModuleID(0),
		123.5,
		Poco::Timespan::SECONDS * 60);

	commandDispatcher->dispatch(cmd, answer);

	ZMQMessage receiveMessage;
	client1->waitOnMessage(6000000, receiveMessage);

	CPPUNIT_ASSERT(receiveMessage.type() == ZMQMessageType::TYPE_SET_VALUES_CMD);
	CPPUNIT_ASSERT(DeviceID(0xa801020304050607UL).prefix() == DevicePrefix::PREFIX_ZWAVE);

	CPPUNIT_ASSERT(queue.wait(1000000, dirtyList));
	CPPUNIT_ASSERT(1 == dirtyList.size());
	CPPUNIT_ASSERT(answer->at(0)->status() == Result::SUCCESS);

	init.stop();
	sleep(1);
}

}
