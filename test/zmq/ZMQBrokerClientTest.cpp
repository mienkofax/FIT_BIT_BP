#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>

#include <Poco/Delegate.h>

#include "core/BasicDistributor.h"
#include "loop/LoopRunner.h"
#include "util/ZMQUtil.h"
#include "zmq/ZMQBroker.h"
#include "zmq/ZMQClient.h"
#include "zmq/ZMQMessage.h"

namespace BeeeOn {

class ZMQBrokerClientTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ZMQBrokerClientTest);
	//CPPUNIT_TEST(testAssignDeviceManagerID);
	CPPUNIT_TEST_SUITE_END();

public:
	void testAssignDeviceManagerID();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ZMQBrokerClientTest);

class FakeClient : public ZMQClient {
public:
	FakeClient():
		ZMQClient()
	{
		onReceive += Poco::delegate(this, &FakeClient::handleMessage);
	}

	void handleMessage(const void*, ZMQMessage& zmqMessage)
	{
		m_event.set();
		m_zmqMessage = zmqMessage;
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
	}

	~InitComponents()
	{
		delete m_broker;

		for (auto item : m_clients)
			delete item;
	}

	FakeBroker* addServer()
	{
		m_broker = new FakeBroker();
		m_broker->setDataServerHost("127.0.0.1");
		m_broker->setDataServerPort(5677);
		m_broker->setHelloServerHost("127.0.0.1");
		m_broker->setHelloServerPort(5678);

		m_broker->setDistributor(m_distributor);
		m_broker->setCommandDispatcher(m_commandDispatcher);
		m_runner.addRunnable(m_broker);

		return m_broker;
	}

	FakeClient* addClient(const DevicePrefix &prefix)
	{
		FakeClient *client = new FakeClient();
		client->setDataServerHost("127.0.0.1");
		client->setDataServerPort(5677);
		client->setHelloServerHost("127.0.0.1");
		client->setHelloServerPort(5678);
		client->setDeviceManagerPrefix(prefix);

		m_runner.addRunnable(client);
		m_clients.push_back(client);

		return client;
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
	FakeBroker *m_broker;
	std::vector<FakeClient *> m_clients;
	Poco::SharedPtr<Distributor> m_distributor;
	Poco::SharedPtr<CommandDispatcher> m_commandDispatcher;
};

/*
 * Overenie, ze po starte clienta sa mu nastavi spravne ID na zaklade
 * device prefixu.
 *
 * Overenie ze odoslana listen sprava prisla bez chyby a moze sa dalej
 * spracovat.
 */
void ZMQBrokerClientTest::testAssignDeviceManagerID()
{
	InitComponents init;

	FakeBroker *broker = init.addServer();
	FakeClient *client1 = init.addClient(DevicePrefix::parse("Z-Wave"));
	FakeClient *client2 = init.addClient(DevicePrefix::parse("Jablotron"));

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
}

}
