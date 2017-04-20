#include "util/JsonUtil.h"
#include "zmq/ZMQMessage.h"

#include "model/SensorData.h"
#include "model/ModuleID.h"
#include "model/DeviceID.h"

using namespace BeeeOn;
using namespace Poco;
using namespace Poco::JSON;
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

Object::Ptr ZMQMessage::jsonObject() const
{
	return m_json;
}

void ZMQMessage::setType(const ZMQMessageType &type)
{
	m_json->set("message_type", type.toString());
}

void ZMQMessage::setErrorCode(const ZMQMessageError::Error &error)
{
	m_json->set("error_code", (int) error);
}

void ZMQMessage::setErrorMessage(const string errorMessage)
{
	m_json->set("error_message", errorMessage);
}

void ZMQMessage::setDeviceManagerPrefix(const DevicePrefix &devicePrefix)
{
	m_json->set("device_manager_prefix", devicePrefix.toString());
}

void ZMQMessage::setDeviceManagerID(const DeviceManagerID &deviceManagerID)
{
	m_json->set("device_manager_id", deviceManagerID.toString());
}

void ZMQMessage::setDeviceID(Object::Ptr jsonObject,
	const DeviceID &deviceID)
{
	jsonObject->set("device_id", deviceID.toString());
}

void ZMQMessage::setModuleID(Object::Ptr jsonObject,
	const ModuleID &moduleID)
{
	jsonObject->set("module_id", moduleID.toString());
}

void ZMQMessage::setRawValue(Object::Ptr jsonObject,
	const std::string &value)
{
	jsonObject->set("raw", value);
}

void ZMQMessage::setRawValueType(Object::Ptr jsonObject,
	 const ZMQMessageValueType &valueType)
{
	jsonObject->set("type", valueType.toString());
}

void ZMQMessage::setValue(Object::Ptr jsonObject, const double value)
{
	ZMQMessageValueType type = ZMQMessageValueType::fromRaw(
		ZMQMessageValueType::TYPE_DOUBLE);

	setRawValue(jsonObject, to_string(value));
	setRawValueType(jsonObject, type);
}

void ZMQMessage::setSensorValue(Object::Ptr jsonObject,
	const SensorValue &sensorValue)
{
	setModuleID(jsonObject, sensorValue.moduleID());
	setValue(jsonObject, sensorValue.value());
}

ZMQMessageError::Error ZMQMessage::getErrorCode()
{
	return static_cast<ZMQMessageError::Error>(
		JsonUtil::extract<int>(m_json, "error_code"));
}

std::string ZMQMessage::getErrorMessage()
{
	return JsonUtil::extract<string>(m_json, "error_message");
}

DevicePrefix ZMQMessage::getDevicePrefix(Poco::JSON::Object::Ptr jsonObject)
{
	return DevicePrefix(DevicePrefix::parse(
		JsonUtil::extract<std::string>(jsonObject, "device_manager_prefix")));
}

DeviceID ZMQMessage::getDeviceID(Object::Ptr jsonObject)
{
	return DeviceID::parse(
		JsonUtil::extract<string>(jsonObject, "device_id"));
}

ModuleID ZMQMessage::getModuleID(Object::Ptr jsonObject)
{
	return ModuleID::parse(
		JsonUtil::extract<string>(jsonObject, "module_id"));
}

string ZMQMessage::getRawValue(Object::Ptr jsonObject)
{
	return JsonUtil::extract<string>(jsonObject, "raw");
}

ZMQMessageValueType ZMQMessage::getRawValueType(Object::Ptr jsonObject)
{
	return ZMQMessageValueType::parse(
		JsonUtil::extract<string>(jsonObject, "type"));
}

bool ZMQMessage::getValue(Object::Ptr jsonObject, double &value)
{
	ZMQMessageValueType type = getRawValueType(jsonObject);

	if (type.raw() != ZMQMessageValueType::TYPE_DOUBLE)
		return false;

	value = NumberParser::parseFloat(getRawValue(jsonObject));
	return true;
}

SensorValue ZMQMessage::getSensorValue(Object::Ptr jsonObject)
{
	double raw;
	ModuleID moduleId = getModuleID(jsonObject);

	if (!getValue(jsonObject, raw))
		return SensorValue(moduleId);

	return SensorValue(moduleId, raw);
}

ZMQMessage ZMQMessage::fromError(
	const ZMQMessageError::Error error, const std::string &message)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_ERROR));
	msg.setErrorCode(error);
	msg.setErrorMessage(message);

	return msg;
}

ZMQMessage ZMQMessage::fromHelloRequest(const DevicePrefix &devicePrefix)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_HELLO_REQUEST));
	msg.setDeviceManagerPrefix(devicePrefix);

	return msg;
}

ZMQMessage ZMQMessage::fromHelloResponse(const DeviceManagerID &deviceManagerID)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_HELLO_RESPONSE));
	msg.setDeviceManagerID(deviceManagerID);

	return msg;
}

ZMQMessage ZMQMessage::fromSensorData(const SensorData &sensorData)
{
	ZMQMessage msg;

	Object::Ptr json = msg.jsonObject();
	Array::Ptr jsonArray = new Array();

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_MEASURED_VALUES));
	msg.setDeviceID(json, sensorData.deviceID());

	for (auto item : sensorData) {
		Object::Ptr arrayItem = new Object();
		msg.setSensorValue(arrayItem, item);
		jsonArray->add(Dynamic::Var(arrayItem));
	}

	json->set("values", jsonArray);

	return msg;
}

ZMQMessage ZMQMessage::fromJSON(const string &json)
{
	return ZMQMessage(JsonUtil::parse(json));
}

ZMQMessageError ZMQMessage::toError()
{
	return ZMQMessageError(getErrorCode(), getErrorMessage());
}

DevicePrefix ZMQMessage::toHelloRequest()
{
	return getDevicePrefix(m_json);
}

DeviceManagerID ZMQMessage::toHelloResponse()
{
	return DeviceManagerID(DeviceManagerID::parse(
		JsonUtil::extract<string>(m_json, "device_manager_id")));
}

SensorData ZMQMessage::toSensorData()
{
	SensorData sensorData;
	sensorData.setDeviceID(getDeviceID(m_json));

	Array::Ptr jsonArray = m_json->getArray("values");
	for (size_t i = 0; i < jsonArray->size(); ++i)
		sensorData.insertValue(getSensorValue(jsonArray->getObject(i)));

	return sensorData;
}
