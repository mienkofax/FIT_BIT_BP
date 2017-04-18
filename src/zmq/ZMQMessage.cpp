#include "util/JsonUtil.h"
#include "zmq/ZMQMessage.h"

using namespace BeeeOn;
using namespace Poco;
using namespace std;

ZMQMessage::ZMQMessage() :
	m_json(new JSON::Object)
{
}

ZMQMessage::ZMQMessage(JSON::Object::Ptr jsonObject)
{
	m_json = jsonObject;
}

ZMQMessageType ZMQMessage::type() const
{
	return ZMQMessageType::parse(
		JsonUtil::extract<string>(m_json, "message_type")
	);
}

std::string ZMQMessage::toString() const
{
	return Dynamic::Var::toString(m_json);
}

void ZMQMessage::setType(const ZMQMessageType &type)
{
	m_json->set("message_type", type.toString());
}

ZMQMessage ZMQMessage::fromJSON(const string &json)
{
	return ZMQMessage(JsonUtil::parse(json));
}
