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

void ZMQMessage::setDuration(const Poco::Timespan &duration)
{
	m_json->set("duration", duration.totalSeconds());
}

void ZMQMessage::setID(const GlobalID &id)
{
	m_json->set("id", id.toString());
}

void ZMQMessage::setResultState(Result::Status resultState)
{
	m_json->set("result_status", (int) resultState);
}

void ZMQMessage::setTimeout(const Poco::Timespan &timeout)
{
	m_json->set("timeout", timeout.totalSeconds());
}

void ZMQMessage::setExtendetSetStatus(DeviceSetValueResult::SetStatus status)
{
	m_json->set("extented_set_status", (int) status);
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

Timespan ZMQMessage::getDuration()
{
	return Timespan(Timespan::SECONDS
		* JsonUtil::extract<int>(m_json, "duration"));
}

DeviceSetValueResult::SetStatus ZMQMessage::getExtendetSetStatus()
{
	return static_cast<DeviceSetValueResult::SetStatus>(
		JsonUtil::extract<int>(m_json, "extented_set_status"));
}

GlobalID ZMQMessage::id()
{
	return GlobalID::parse(JsonUtil::extract<string>(m_json, "id"));
}

Result::Status ZMQMessage::getResultState()
{
	return static_cast<Result::Status>(
		JsonUtil::extract<int>(m_json, "result_status"));
}

Poco::Timespan ZMQMessage::getTimeout(Poco::JSON::Object::Ptr jsonObject)
{
	return Timespan(Timespan::SECONDS
		* JsonUtil::extract<int>(jsonObject, "timeout"));
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

ZMQMessage ZMQMessage::fromCommand(const Command::Ptr cmd)
{
	if (cmd->is<GatewayListenCommand>()) {
		return fromGatewayListenCommand(cmd.cast<GatewayListenCommand>());
	}
	else if (cmd->is<DeviceSetValueCommand>()) {
		return fromDeviceSetValueCommand(cmd.cast<DeviceSetValueCommand>());
	}
	else if (cmd->is<ServerDeviceListCommand>()) {
		return fromServerDeviceListCommand(cmd.cast<ServerDeviceListCommand>());
	}
	else if (cmd->is<ServerLastValueCommand>()) {
		return fromServerLastValueCommand(cmd.cast<ServerLastValueCommand>());
	}
	else if (cmd->is<DeviceUnpairCommand>()) {
		return fromDeviceUnpairCommand(cmd.cast<DeviceUnpairCommand>());
	}
	else {
		throw Poco::ExistsException("unsupported command: "
			+ cmd->name());
	}
}

ZMQMessage ZMQMessage::fromGatewayListenCommand(const GatewayListenCommand::Ptr cmd)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_LISTEN_CMD));
	msg.setID(GlobalID::random());
	msg.setDuration(cmd->duration());

	return msg;
}

ZMQMessage ZMQMessage::fromResult(const Result::Ptr result)
{
	if (result->is<Result>()) {
		return fromDefaultResult(result.cast<Result>());
	}
	else if (result->is<ServerDeviceListResult>()) {
		return fromServerDeviceListResult(result.cast<ServerDeviceListResult>());
	}
	else if (result->is<ServerLastValueResult>()) {
		return fromServerLastValueResult(result.cast<ServerLastValueResult>());
	}
	else if (result->is<DeviceSetValueResult>()) {
		return fromDeviceSetValueResult(result.cast<DeviceSetValueResult>());
	}
	else {
		throw Poco::ExistsException("unsupported result");
	}
}

ZMQMessage ZMQMessage::fromDefaultResult(const Result::Ptr result)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_DEFAULT_RESULT	));
	msg.setResultState(result->status());

	return msg;
}

ZMQMessage ZMQMessage::fromDeviceSetValueCommand(const DeviceSetValueCommand::Ptr cmd)
{
	Object::Ptr values = new Object();

	ZMQMessage msg;
	msg.setID(GlobalID::random());

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_SET_VALUES_CMD));

	msg.setTimeout(cmd->timeout());
	msg.setValue(values, cmd->value());
	msg.setModuleID(values, cmd->moduleID());
	msg.setDeviceID(msg.jsonObject(), cmd->deviceID());

	msg.jsonObject()->set("values", values);

	return msg;
}

ZMQMessage ZMQMessage::fromServerDeviceListCommand(
	const ServerDeviceListCommand::Ptr cmd)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_DEVICE_LIST_CMD));
	msg.setDeviceManagerPrefix(cmd->devicePrefix());

	return msg;
}

ZMQMessage ZMQMessage::fromServerDeviceListResult(const ServerDeviceListResult::Ptr result)
{
	ZMQMessage msg;

	Object::Ptr json = msg.jsonObject();
	Array::Ptr jsonArray = new Array();

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_DEVICE_LIST_RESULT));
	msg.setResultState(result->status());

	for (auto item : result->deviceList()) {
		Object::Ptr arrayItem = new Object();
		msg.setDeviceID(arrayItem, item);
		jsonArray->add(Dynamic::Var(arrayItem));
	}

	json->set("device_list", jsonArray);

	return msg;
}

ZMQMessage ZMQMessage::fromServerLastValueCommand(
	const ServerLastValueCommand::Ptr cmd)
{
	ZMQMessage msg;
	Object::Ptr json = msg.jsonObject();

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_DEVICE_LAST_VALUE_CMD));
	msg.setDeviceID(json, cmd->deviceID());
	msg.setModuleID(json, cmd->moduleID());

	return msg;
}


ZMQMessage ZMQMessage::fromServerLastValueResult(
	const ServerLastValueResult::Ptr result)
{
	Object::Ptr values = new Object();
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_DEVICE_LAST_VALUE_RESULT));
	msg.setValue(values, result->value());
	msg.setResultState(result->status());
	msg.jsonObject()->set("values", values);

	return msg;
}

ZMQMessage ZMQMessage::fromDeviceUnpairCommand(const DeviceUnpairCommand::Ptr cmd)
{
	ZMQMessage msg;
	Object::Ptr json = msg.jsonObject();

	msg.setID(GlobalID::random());
	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_DEVICE_UNPAIR_CMD));
	msg.setDeviceID(json, cmd->deviceID());

	return msg;
}

ZMQMessage ZMQMessage::fromDeviceSetValueResult(
	const DeviceSetValueResult::Ptr result)
{
	ZMQMessage msg;

	msg.setType(ZMQMessageType::fromRaw(
		ZMQMessageType::TYPE_SET_VALUES_RESULT));
	msg.setResultState(result->status());
	msg.setExtendetSetStatus(result->extendetSetStatus());

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

GatewayListenCommand::Ptr ZMQMessage::toGatewayListenCommand()
{
	return new GatewayListenCommand(getDuration());
}

void ZMQMessage::toDefaultResult(Result::Ptr result)
{
	result->setStatus(getResultState());
}

DeviceSetValueCommand::Ptr ZMQMessage::toDeviceSetValueCommand()
{
	Object::Ptr jsonObject = m_json->getObject("values");
	double raw;

	if (!getValue(jsonObject, raw))
		throw InvalidArgumentException("invalid set message");

	return new DeviceSetValueCommand(
		getDeviceID(m_json),
		getModuleID(jsonObject),
		raw,
		getTimeout(m_json)
	);
}

ServerDeviceListCommand::Ptr ZMQMessage::toDeviceListRequest()
{
	return new ServerDeviceListCommand(DevicePrefix::parse(
		JsonUtil::extract<std::string>((m_json), "device_manager_prefix")));
}

void ZMQMessage::toServerDeviceListResult(ServerDeviceListResult::Ptr result)
{
	vector<DeviceID> deviceList;

	Array::Ptr jsonArray = m_json->getArray("device_list");
	for (size_t i = 0; i < jsonArray->size(); ++i)
		deviceList.push_back(getDeviceID(jsonArray->getObject(i)));

	result->setStatus(getResultState());
	result->setDeviceList(deviceList);
}

ServerLastValueCommand::Ptr ZMQMessage::toServerLastValueCommand()
{
	return new ServerLastValueCommand(getDeviceID(m_json), getModuleID(m_json));
}

void ZMQMessage::toServerLastValueResult(ServerLastValueResult::Ptr result)
{
	Object::Ptr jsonObject = m_json->getObject("values");
	double raw;

	if (!getValue(jsonObject, raw))
		throw InvalidArgumentException("invalid set message");

	result->setStatus(getResultState());
	result->setValue(raw);
}

DeviceUnpairCommand::Ptr ZMQMessage::toDeviceUnpairCommand()
{
	return new DeviceUnpairCommand(getDeviceID(m_json));
}

void ZMQMessage::toDeviceSetValueResult(DeviceSetValueResult::Ptr result)
{
	result->setStatus(getResultState());
	result->setExtendetSetStatus((getExtendetSetStatus()));
}
