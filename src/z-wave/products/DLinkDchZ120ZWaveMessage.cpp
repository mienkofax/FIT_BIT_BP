#include <Poco/NumberParser.h>

#include "z-wave/products/DLinkDchZ120ZWaveMessage.h"

#define PIR_SENSOR_SENSITIVITY_MODULE  3
#define PIR_SENSOR_SENSITIVITY_INDEX   3

#define ALARM_MOTION       8
#define ALARM_SAFETY       3
#define ALARM_IDLE         0

#define MODULE_PIR              0
#define MODULE_LUMINANCE        1
#define MODULE_TEMPERATURE      2
#define MODULE_SENSOR_ALARM     3
#define MODULE_PIR_SENSITIVITY  4
#define MODULE_BATTERY          5

using namespace BeeeOn;
using std::string;
using std::vector;

SensorData DLinkDchZ120ZWaveMessage::extractValues(
	const vector<ZWaveSensorValue> &zwaveValues)
{
	SensorData sensorData;
	string value;

	if (getLuminance(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_LUMINANCE),
			Poco::NumberParser::parseFloat(value)));

	if (getTemperature(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_TEMPERATURE),
			Poco::NumberParser::parseFloat(value)));

	if (getPirSensitivity(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_PIR_SENSITIVITY),
			Poco::NumberParser::parseFloat(value)));

	if (getBatteryLevel(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_BATTERY),
			Poco::NumberParser::parseFloat(value)));

	getDetectionSensor(sensorData, zwaveValues);

	return sensorData;
}

void DLinkDchZ120ZWaveMessage::setValue(const SensorData &sensorData,
	const uint8_t &nodeId)
{
	string value;

	for (auto data : sensorData) {
		if (data.moduleID().value() == MODULE_PIR_SENSITIVITY)
			setActuator(std::to_string(data.value()), COMMAND_CLASS_CONFIGURATION,
				PIR_SENSOR_SENSITIVITY_INDEX, nodeId);
	}
}

bool DLinkDchZ120ZWaveMessage::getPirSensitivity(std::string &value,
	const std::vector<ZWaveSensorValue> &zwaveValues)
{
	return getSpecificValue(value, COMMAND_CLASS_CONFIGURATION,
	PIR_SENSOR_SENSITIVITY_INDEX, zwaveValues);
}

void DLinkDchZ120ZWaveMessage::getDetectionSensor(SensorData &sensorData,
	const vector<ZWaveSensorValue> &zwaveValues)
{
	int sensorState = 0;

	for (const ZWaveSensorValue &item : zwaveValues) {
		if (!isEqual(item.commandClass, COMMAND_CLASS_ALARM, item.index,
				SENSOR_INDEX_BULGAR))
			continue;

		if (!extractInt(sensorState, item))
			continue;

		if (sensorState == ALARM_SAFETY) {
			sensorData.insertValue(SensorValue(ModuleID(MODULE_SENSOR_ALARM), 1));
		} else if (sensorState == ALARM_MOTION) {
			sensorData.insertValue(SensorValue(ModuleID(MODULE_PIR), 1));
		} else {
			sensorData.insertValue(SensorValue(ModuleID(MODULE_PIR), 0));
			sensorData.insertValue(SensorValue(ModuleID(MODULE_SENSOR_ALARM), 0));
		}
	}
}

void DLinkDchZ120ZWaveMessage::setAfterStart()
{
}

int DLinkDchZ120ZWaveMessage::getDeviceID()
{
	return DEVICE_ID_DLINK_DCH_Z120;
}
