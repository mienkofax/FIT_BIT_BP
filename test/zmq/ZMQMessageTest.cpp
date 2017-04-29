#include <cppunit/extensions/HelperMacros.h>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "commands/DeviceSetValueCommand.h"
#include "commands/GatewayListenCommand.h"
#include "core/AnswerQueue.h"
#include "model/SensorData.h"
#include "util/JsonUtil.h"
#include "zmq/ZMQMessage.h"
#include "model/GlobalID.h"

using namespace Poco;
using namespace Poco::JSON;
using namespace std;

namespace BeeeOn {

class ZMQMessageTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ZMQMessageTest);
	CPPUNIT_TEST(testParseFromJSON);
	CPPUNIT_TEST(testErrorMessage);
	CPPUNIT_TEST(testHelloRequest);
	CPPUNIT_TEST(testHelloResponse);
	CPPUNIT_TEST(testMeasuredValues);
	CPPUNIT_TEST(testGatewayListenCommand);
	CPPUNIT_TEST(testDefaultResult);
	CPPUNIT_TEST(testDeviceSetValueCommand);
	CPPUNIT_TEST(testDeviceListCommand);
	CPPUNIT_TEST(testDeviceListResult);
	CPPUNIT_TEST(testServerLastValueCommand);
	CPPUNIT_TEST(testServerLastValueResult);
	CPPUNIT_TEST(testDeviceUnpairCommand);
	CPPUNIT_TEST_SUITE_END();

public:
	void testParseFromJSON();
	void testErrorMessage();
	void testHelloRequest();
	void testHelloResponse();
	void testMeasuredValues();
	void testGatewayListenCommand();
	void testDefaultResult();
	void testDeviceSetValueCommand();
	void testDeviceListCommand();
	void testDeviceListResult();
	void testServerLastValueCommand();
	void testServerLastValueResult();
	void testDeviceUnpairCommand();

private:
	std::string toPocoJSON(const std::string &json);
};

CPPUNIT_TEST_SUITE_REGISTRATION(ZMQMessageTest);

/*
 * Convert string json to Poco::JSON string format.
 */
std::string ZMQMessageTest::toPocoJSON(const std::string &json)
{
	return Dynamic::Var::toString(JsonUtil::parse(json));
}

/*
 * It tests the message parsing and gets type of message. It tries to choose
 * an absent attribute with exception and parses invalid json message
 * with exception.
 */
void ZMQMessageTest::testParseFromJSON()
{
	string jsonMessage = R"(
		{
			"message_type" : "error"
		}
	)";

	ZMQMessage message = ZMQMessage::fromJSON(jsonMessage);
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_ERROR);

	message = ZMQMessage::fromJSON("{}");
	CPPUNIT_ASSERT_THROW(message.type(), Poco::InvalidAccessException);

	CPPUNIT_ASSERT_THROW(
		ZMQMessage::fromJSON("invalidJSON"), Poco::SyntaxException);
}

/*
 * It compares created message with origin message.
 * It tests the message parsing and obtains type of message
 * (error).
 */
void ZMQMessageTest::testErrorMessage()
{
	string jsonMessage = R"(
		{
			"message_type" : "error",
			"error_code" : 0,
			"error_message" : "unknown error"
		}
	)";

	ZMQMessage message =
		ZMQMessage::fromError(
			static_cast<ZMQMessageError::Error>(0),
			"unknown error");

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_ERROR);

	ZMQMessageError error = message.toError();
	CPPUNIT_ASSERT(error.errorMessage() == "unknown error");
	CPPUNIT_ASSERT_EQUAL(error.errorCode(),
		static_cast<ZMQMessageError::Error>(0));
}

void ZMQMessageTest::testHelloRequest()
{
	string jsonMessage = R"(
		{
			"message_type" : "hello_request",
			"device_manager_prefix" : "Fitprotocol"
		}
	)";

	ZMQMessage message = ZMQMessage::fromHelloRequest(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_HELLO_REQUEST);

	DevicePrefix helloRequest = message.toHelloRequest();
	CPPUNIT_ASSERT_EQUAL(
		helloRequest,
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL));
}

void ZMQMessageTest::testHelloResponse()
{
	DeviceManagerID deviceManagerID(0xa100);
	string jsonMessage = R"(
		{
			"message_type" : "hello_response",
			"device_manager_id" : "A100"
		}
	)";

	ZMQMessage message = ZMQMessage::fromHelloResponse(deviceManagerID);

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_HELLO_RESPONSE);

	DeviceManagerID helloResponse = message.toHelloResponse();
	CPPUNIT_ASSERT_EQUAL(helloResponse, deviceManagerID);
}

void ZMQMessageTest::testMeasuredValues()
{
	string jsonMessage = R"(
		{
			"device_id" : "0xfe01020304050607",
			"message_type" : "measured_values",
			"values" : [
				{
					"module_id" : "0",
					"raw" : "123.500000",
					"type" : "double"
				},
				{
					"module_id" : "1",
					"raw" : "-59.400000",
					"type" : "double"
				}
			]
		}
	)";
	Timestamp now;
	SensorData testSensorData;

	testSensorData.setDeviceID(DeviceID(0xfe01020304050607));
	testSensorData.insertValue(SensorValue(ModuleID(0), 123.5));
	testSensorData.insertValue(SensorValue(ModuleID(1), -59.4));
	testSensorData.setTimestamp(now);

	ZMQMessage message = ZMQMessage::fromSensorData(testSensorData);

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_MEASURED_VALUES);

	SensorData sensorData = message.toSensorData();
	sensorData.setTimestamp(now);

	CPPUNIT_ASSERT(testSensorData == sensorData);
}

void ZMQMessageTest::testGatewayListenCommand()
{
	Timespan duration(60*Timespan::SECONDS);
	string jsonMessage = R"(
		{
			"message_type" : "listen_cmd",
			"duration" : 60,
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d"
		}
	)";

	GatewayListenCommand::Ptr listen = new GatewayListenCommand(duration);
	ZMQMessage message = ZMQMessage::fromCommand(listen);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT("3feca65f-fdfc-4189-ad9d-0be68e13ef5d" == message.id().toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_LISTEN_CMD);

	GatewayListenCommand::Ptr cmd = message.toGatewayListenCommand();
	CPPUNIT_ASSERT(cmd->duration() == duration);
}

void ZMQMessageTest::testDefaultResult()
{
	string jsonMessage = R"(
		{
			"message_type" : "default_result",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d",
			"result_status" : 0
		}
	)";

	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	Result::Ptr result = new Result(answer);

	ZMQMessage message = ZMQMessage::fromResult(result);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_DEFAULT_RESULT);

	message.toDefaultResult(result);
	CPPUNIT_ASSERT(result->status() == static_cast<Result::Status>(0));
}

void ZMQMessageTest::testDeviceSetValueCommand()
{
	string jsonMessage = R"(
		{
			"device_id" : "0xfe01020304050607",
			"message_type" : "set_values_cmd",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d",
			"timeout" : 60,
			"values" : {
					"module_id" : "0",
					"raw" : "123.500000",
					"type" : "double"
			}
		}
	)";

	DeviceSetValueCommand::Ptr set = new DeviceSetValueCommand(
		DeviceID(0xfe01020304050607),
		ModuleID(0),
		123.5,
		Timespan::SECONDS * 60);

	ZMQMessage message = ZMQMessage::fromCommand(set);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_SET_VALUES_CMD);

	DeviceSetValueCommand::Ptr cmd = message.toDeviceSetValueCommand();
	CPPUNIT_ASSERT(cmd->deviceID() == DeviceID(0xfe01020304050607));
	CPPUNIT_ASSERT(cmd->timeout().totalSeconds() == 60);
	CPPUNIT_ASSERT(cmd->moduleID() == ModuleID(0));
	CPPUNIT_ASSERT(cmd->value() == 123.5);
}

void ZMQMessageTest::testDeviceListCommand()
{
	string jsonMessage = R"(
		{
			"message_type" : "device_list_cmd",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d",
			"device_manager_prefix" : "Fitprotocol"
		}
	)";

	ServerDeviceListCommand::Ptr deviecList = new ServerDeviceListCommand(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL));

	ZMQMessage message = ZMQMessage::fromCommand(deviecList);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_DEVICE_LIST_CMD);

	ServerDeviceListCommand::Ptr cmd = message.toDeviceListRequest();
	CPPUNIT_ASSERT_EQUAL(
		cmd->devicePrefix(),
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL));
}

void ZMQMessageTest::testDeviceListResult()
{
	string jsonMessage = R"(
		{
			"device_list" : [
				{
					"device_id" : "0xfe01020304050607"
				},
				{
					"device_id" : "0xfe01020304050608"
				},
				{
					"device_id" : "0xfe01020304050609"
				}
			],
			"result_status" : 0,
			"message_type" : "device_list_result",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d"
		}
	)";

	vector<DeviceID> devices = {
		DeviceID(0xfe01020304050607),
		DeviceID(0xfe01020304050608),
		DeviceID(0xfe01020304050609),
	};

	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	ServerDeviceListResult::Ptr result = new ServerDeviceListResult(answer);
	result->setDeviceList(devices);

	ZMQMessage message = ZMQMessage::fromResult(result);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));
	
	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_DEVICE_LIST_RESULT);

	message.toServerDeviceListResult(result);
	CPPUNIT_ASSERT(result->status() == static_cast<Result::Status>(0));
	CPPUNIT_ASSERT(result->deviceList() == devices);
}

void ZMQMessageTest::testServerLastValueCommand()
{
	DeviceID deviceID(0xfe01020304050607);
	ModuleID moduleID(0);

	string jsonMessage = R"(
		{
			"message_type" : "device_last_value_cmd",
			"device_id" : "0xfe01020304050607",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d",
			"module_id" : "0"
		}
	)";

	ServerLastValueCommand::Ptr listen =
		new ServerLastValueCommand(deviceID, moduleID);
	ZMQMessage message = ZMQMessage::fromCommand(listen);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_DEVICE_LAST_VALUE_CMD);

	ServerLastValueCommand::Ptr cmd = message.toServerLastValueCommand();
	CPPUNIT_ASSERT(cmd->deviceID() == deviceID);
	CPPUNIT_ASSERT(cmd->moduleID() == moduleID);
}

void ZMQMessageTest::testServerLastValueResult()
{
	string jsonMessage = R"(
		{
			"message_type" : "device_last_value_result",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d",
			"result_status" : 0,
			"values" : {
					"raw" : "123.500000",
					"type" : "double"
			}
		}
	)";

	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	ServerLastValueResult::Ptr result = new ServerLastValueResult(answer);
	result->setValue(123.5);

	ZMQMessage message = ZMQMessage::fromResult(result);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_DEVICE_LAST_VALUE_RESULT);

	message.toServerLastValueResult(result);
	CPPUNIT_ASSERT(result->value() == 123.5);
}

void ZMQMessageTest::testDeviceUnpairCommand()
{
	DeviceID deviceID(0xfe01020304050607);

	string jsonMessage = R"(
		{
			"message_type" : "device_unpair_cmd",
			"id" : "3feca65f-fdfc-4189-ad9d-0be68e13ef5d",
			"device_id" : "0xfe01020304050607"
		}
	)";

	DeviceUnpairCommand::Ptr listen =
		new DeviceUnpairCommand(deviceID);
	ZMQMessage message = ZMQMessage::fromCommand(listen);
	message.setID(GlobalID::parse("3feca65f-fdfc-4189-ad9d-0be68e13ef5d"));

	CPPUNIT_ASSERT(toPocoJSON(jsonMessage) == message.toString());
	CPPUNIT_ASSERT(message.type() == ZMQMessageType::TYPE_DEVICE_UNPAIR_CMD);

	DeviceUnpairCommand::Ptr cmd = message.toDeviceUnpairCommand();
	CPPUNIT_ASSERT(cmd->deviceID() == deviceID);
}

}
