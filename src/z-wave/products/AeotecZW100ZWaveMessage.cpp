#include <vector>

#include <Poco/NumberParser.h>

#include "z-wave/products/AeotecZW100ZWaveMessage.h"

#define PIR_SENSOR_ACTUATOR        6
#define PIR_SENSOR_INDEX           4
#define SENSOR_PIR_INTENSITY       2
#define SENSOR_INDEX_REFRESH_TIME  111

#define SHAKE    3
#define MOTION   8
#define IDLE     0

#define MODULE_SHAKE_SENSOR            0
#define MODULE_PIR_SENSOR              1
#define MODULE_ULTRAVIOLET             2
#define MODULE_LIGHT                   3
#define MODULE_ROOM_TEMPERATURE        4
#define MODULE_ROOM_HUMIDITY           5
#define MODULE_PIR_SENSOR_SENSITIVITY  6
#define MODULE_BATTERY                 7
#define MODULE_REFRESH_TIME            8

using namespace BeeeOn;
using std::string;
using std::to_string;
using std::vector;

AeotecZW100ZWaveMessage::AeotecZW100ZWaveMessage()
{
	setMapValue();
}

SensorData AeotecZW100ZWaveMessage::extractValues(
	const vector<ZWaveSensorValue> &zwaveValues)
{
	SensorData sensorData;
	string value;

	if (getUltraviolet(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_ULTRAVIOLET),
			Poco::NumberParser::parseFloat(value)));

	if (getLuminance(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_LIGHT),
			Poco::NumberParser::parseFloat(value)));

	if (getTemperature(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_ROOM_TEMPERATURE),
			Poco::NumberParser::parseFloat(value)));

	if (getHumidity(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_ROOM_HUMIDITY),
			Poco::NumberParser::parseFloat(value)));

	if (getPirSensitivity(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_PIR_SENSOR_SENSITIVITY),
			Poco::NumberParser::parseFloat(value)
		));

	if (getBatteryLevel(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_BATTERY),
		Poco::NumberParser::parseFloat(value)));

	if (getRefreshTime(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_REFRESH_TIME),
			Poco::NumberParser::parseFloat(value)));

	getDetectionSensor(sensorData, zwaveValues);

	return sensorData;
}

void AeotecZW100ZWaveMessage::setValue(const SensorData &sensorData,
	const uint8_t &nodeId)
{
	std::string value;
	for (auto data : sensorData) {
		if (data.moduleID().value() == PIR_SENSOR_ACTUATOR) {
			int actuatorValue = int(data.value());
			auto search = m_pirSensor.find(to_string(actuatorValue));

			if (search == m_pirSensor.end())
				return;

			setActuator(search->second, COMMAND_CLASS_CONFIGURATION,
				PIR_SENSOR_INDEX, nodeId);
		}
		else if (data.moduleID().value() == MODULE_REFRESH_TIME) {
			setActuator(std::to_string(data.value()), COMMAND_CLASS_CONFIGURATION,
				SENSOR_INDEX_REFRESH_TIME, nodeId);
		}
	}
}

bool AeotecZW100ZWaveMessage::getPirSensitivity(string& value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	if (!getSpecificValue(value, COMMAND_CLASS_CONFIGURATION,
		PIR_SENSOR_INDEX, zwaveValues))
		return false;

	for (const auto &item : m_pirSensor) {
		if (item.second == value) {
			value = item.first;
			return true;
		}
	}

	return false;
}

bool AeotecZW100ZWaveMessage::getRefreshTime(string& value,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	return getSpecificValue(value, COMMAND_CLASS_CONFIGURATION,
	SENSOR_INDEX_REFRESH_TIME, zwaveValues);
}

void AeotecZW100ZWaveMessage::getDetectionSensor(SensorData &sensorData,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	int sensorState = 0;

	for (const ZWaveSensorValue &item : zwaveValues) {
		if (!isEqual(item.commandClass, COMMAND_CLASS_ALARM, item.index,
				SENSOR_INDEX_BULGAR))
			continue;

		if (!extractInt(sensorState, item))
			continue;

		if (sensorState == SHAKE) {
			sensorData.insertValue(SensorValue(ModuleID(MODULE_SHAKE_SENSOR), 1));
		} else if (sensorState == MOTION) {
			sensorData.insertValue(SensorValue(ModuleID(MODULE_PIR_SENSOR), 1));
		} else {
			sensorData.insertValue(SensorValue(ModuleID(MODULE_SHAKE_SENSOR), 0));
			sensorData.insertValue(SensorValue(ModuleID(MODULE_PIR_SENSOR), 0));
		}
	}
}

void AeotecZW100ZWaveMessage::setMapValue()
{
	m_pirSensor.insert(std::make_pair("0", "Disabled"));
	m_pirSensor.insert(std::make_pair("1", "Enabled level 1 (minimum sensitivity)"));
	m_pirSensor.insert(std::make_pair("2", "Enabled level 2"));
	m_pirSensor.insert(std::make_pair("3", "Enabled level 3"));
	m_pirSensor.insert(std::make_pair("4", "Enabled level 4"));
	m_pirSensor.insert(std::make_pair("5", "Enabled level 5 (maximum sensitivity)"));
}

int AeotecZW100ZWaveMessage::getDeviceID()
{
	return DEVICE_ID_AEOTEC_ZW100;
}

void AeotecZW100ZWaveMessage::setAfterStart()
{
}
