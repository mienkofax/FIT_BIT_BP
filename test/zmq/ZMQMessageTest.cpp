#include <cppunit/extensions/HelperMacros.h>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "util/JsonUtil.h"
#include "zmq/ZMQMessage.h"

using namespace Poco;
using namespace Poco::JSON;
using namespace std;

namespace BeeeOn {

class ZMQMessageTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ZMQMessageTest);
	CPPUNIT_TEST(testParseFromJSON);
	CPPUNIT_TEST_SUITE_END();

public:
	void testParseFromJSON();

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

}
