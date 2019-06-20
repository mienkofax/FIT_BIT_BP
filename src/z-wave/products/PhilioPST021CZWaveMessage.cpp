#include <Poco/NumberParser.h>

#include "z-wave/products/PhilioPST021CZWaveMessage.h"

#define ALARM_OPEN         8
#define ALARM_SAFETY       3
#define ALARM_IDLE         0

#define MODULE_MAGNETIC_CONTACT     0
#define MODULE_TEMPERATURE          1
#define MODULE_LUMINANCE            2
#define MODULE_BATTERY              3

using namespace BeeeOn;
using std::string;
using std::vector;

SensorData PhilioPST021CZWaveMessage::extractValues(
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

	if (getSensorValue(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_MAGNETIC_CONTACT),
			Poco::NumberParser::parseFloat(value)));

	if (getBatteryLevel(value, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_BATTERY),
			Poco::NumberParser::parseFloat(value)));

	return sensorData;
}

void PhilioPST021CZWaveMessage::setValue(const SensorData&, const uint8_t&)
{
}

void PhilioPST021CZWaveMessage::setAfterStart()
{
}

int PhilioPST021CZWaveMessage::getDeviceID()
{
	return DEVICE_ID_PHILIO_PST02_1C;
}
