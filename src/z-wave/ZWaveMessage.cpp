#include <list>

#include <Poco/Exception.h>
#include <Poco/Logger.h>
#include <Poco/NumberParser.h>

#include <Manager.h>

#include "z-wave/ZWaveMessage.h"
#include "z-wave/NotificationProcessor.h"

using namespace BeeeOn;
using namespace std;

#define SENSOR_INDEX_BATTERY             0
#define SENSOR_INDEX_HUMINIDITY          5
#define SENSOR_INDEX_LUMINISTANCE        3
#define SENSOR_INDEX_SENSOR              0
#define SENSOR_INDEX_TEMPERATURE         1
#define SENSOR_INDEX_ULTRAVIOLET         27

bool ZWaveMessage::asBool(const string &value)
{
	return (bool) Poco::NumberParser::parseFloat(value);
}

bool ZWaveMessage::extractFloat(double &value, const ZWaveSensorValue &item)
{
	try {
		value = Poco::NumberParser::parseFloat(item.value);;
		return true;
	}
	catch (Poco::Exception& ex) {
		logger().error("Failed to parse value " + item.value + "as a double");
		logger().log(ex, __FILE__, __LINE__);
		return false;
	}
}

bool ZWaveMessage::extractInt(int &value, const ZWaveSensorValue &item)
{
	try {
		value = Poco::NumberParser::parse(item.value);;
		return true;
	}
	catch (Poco::Exception& ex) {
		logger().error("Failed to parse value " + item.value + "as a int");
		logger().log(ex, __FILE__, __LINE__);
		return false;
	}
}

bool ZWaveMessage::extractBool(bool &value, const ZWaveSensorValue &item)
{
	if (item.value == "True") {
		value = true;
		return true;
	} else if (item.value == "False") {
		value = false;
		return true;
	}
	else {
		return false;
	}
}

bool ZWaveMessage::extractString(string &value,
	const ZWaveSensorValue &item)
{
	value = item.value;
	return true;
}

bool ZWaveMessage::getSpecificValue(string &value, const int &commandClass,
	const int &index, const vector<ZWaveSensorValue> &zwaveValues)
{
	for (const ZWaveSensorValue &item : zwaveValues) {
		if (!isEqual(item.commandClass,
				commandClass,
				item.index,
				index))
			continue;

		value = item.value;
		return true;
	}

	return false;
}

bool ZWaveMessage::getTemperature(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	double temperature;

	for (const ZWaveSensorValue &item : zwaveValues) {
		if (!isEqual(item.commandClass,
				COMMAND_CLASS_SENSOR_MULTILEVEL,
				item.index,
				SENSOR_INDEX_TEMPERATURE))
			continue;

		if (!extractFloat(temperature, item))
			continue;

		if (item.unit == "F")
			temperature = (temperature - 32)/1.8;

		value = to_string(temperature);
		return true;
	}

	return false;
}

bool ZWaveMessage::getSwitchBinary(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	bool switchValue;

	for (const ZWaveSensorValue &item : zwaveValues) {
		if (!isEqual(item.commandClass,
				COMMAND_CLASS_SWITCH_BINARY,
				item.index,
				SENSOR_INDEX_SENSOR))
			continue;

		if (!extractBool(switchValue, item))
			continue;

		value = to_string(switchValue);
		return true;
	}

	return false;
}

bool ZWaveMessage::getBatteryLevel(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	return getSpecificValue(value,
		COMMAND_CLASS_BATTERY,
		SENSOR_INDEX_BATTERY,
		zwaveValues);
}

bool ZWaveMessage::getSensorValue(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	bool sensorValue;

	for (const ZWaveSensorValue &item : zwaveValues) {
		if (!isEqual(item.commandClass,
				COMMAND_CLASS_SENSOR_BINARY,
				item.index,
				SENSOR_INDEX_SENSOR))
			continue;

		if (!extractBool(sensorValue, item))
			continue;

		value = to_string(sensorValue);
		return true;
	}

	return false;
}

bool ZWaveMessage::getLuminance(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	return getSpecificValue(value, COMMAND_CLASS_SENSOR_MULTILEVEL,
			SENSOR_INDEX_LUMINISTANCE, zwaveValues);
}

bool ZWaveMessage::getHumidity(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	return getSpecificValue(value, COMMAND_CLASS_SENSOR_MULTILEVEL,
			SENSOR_INDEX_HUMINIDITY, zwaveValues);
}

bool ZWaveMessage::getUltraviolet(string &value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	return getSpecificValue(value, COMMAND_CLASS_SENSOR_MULTILEVEL,
			SENSOR_INDEX_ULTRAVIOLET, zwaveValues);
}

void ZWaveMessage::sendActuatorValue(const OpenZWave::ValueID &valueId,
	const string &value)
{
	double data = 0;
	int valueType = valueId.GetType();

	try {
		switch(valueType) {
		case OpenZWave::ValueID::ValueType_Bool:
			OpenZWave::Manager::Get()->SetValue(valueId, asBool(value));
			break;
		case OpenZWave::ValueID::ValueType_Byte:
			data = Poco::NumberParser::parseFloat(value);
			OpenZWave::Manager::Get()->SetValue(valueId, uint8_t(data));
			break;
		case OpenZWave::ValueID::ValueType_Short:
			data = Poco::NumberParser::parseFloat(value);
			OpenZWave::Manager::Get()->SetValue(valueId, int16_t(data));
			break;
		case OpenZWave::ValueID::ValueType_Int:
			data = Poco::NumberParser::parseFloat(value);
			OpenZWave::Manager::Get()->SetValue(valueId, int(data));
			break;
		case OpenZWave::ValueID::ValueType_List:
			OpenZWave::Manager::Get()->SetValueListSelection(valueId, value);
			break;
		default:
			logger().error("Unsupported ValueID " + to_string(valueType),
					__FILE__, __LINE__);
			break;
		}
	} catch (Poco::Exception &ex) {
		logger().error("Failed to parse value " +
			to_string(data) + " as a float");
		logger().log(ex, __FILE__, __LINE__);
	}
}

bool ZWaveMessage::setActuator(const std::string &value, const int &commandClass,
	const int &index, const uint8_t &nodeId)
{
	NotificationProcessor::findNodeInfo(nodeId);
	Poco::Nullable<NodeInfo> nodeInfo =
		NotificationProcessor::findNodeInfo(nodeId);

	if (nodeInfo.isNull())
		return false;

	for (const OpenZWave::ValueID &item : nodeInfo.value().m_values) {
		if (item.GetCommandClassId() == commandClass && item.GetIndex() == index) {
			logger().debug("Set actuator, commandClass " + to_string(commandClass)
					+ " index " + to_string(index));
			sendActuatorValue(item, value);
			return true;
		}
	}

	return false;
}

bool ZWaveMessage::isEqual(const int &commandClass1,
	const int &commandClass2, const int &index1, const int &index2) const
{
	return commandClass1 == commandClass2 && index1 == index2;
}
