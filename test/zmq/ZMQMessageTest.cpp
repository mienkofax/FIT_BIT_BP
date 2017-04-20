#include <cppunit/extensions/HelperMacros.h>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "model/SensorData.h"
#include "util/JsonUtil.h"
#include "zmq/ZMQMessage.h"

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
	CPPUNIT_TEST_SUITE_END();

public:
	void testParseFromJSON();
	void testErrorMessage();
	void testHelloRequest();
	void testHelloResponse();
	void testMeasuredValues();

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

}
