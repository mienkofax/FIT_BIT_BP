#include <string>
#include <vector>

#include <Poco/NumberParser.h>

#include "z-wave/products/FibaroFGK107ZWaveMessage.h"

#define MODULE_MAGNETIC_DOOR_CONTACT  0
#define MODULE_BATTERY                1

using namespace BeeeOn;
using std::string;
using std::vector;

SensorData FibaroFGK107ZWaveMessage::extractValues(
	const vector<ZWaveSensorValue> &zwaveValues)
{
	string data;
	SensorData sensorData;

	if (getSensorValue(data, zwaveValues)) {
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_MAGNETIC_DOOR_CONTACT),
			Poco::NumberParser::parseFloat(data)));

	}

	if (getBatteryLevel(data, zwaveValues)) {
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_BATTERY),
			Poco::NumberParser::parseFloat(data)));
	}

	return sensorData;
}

void FibaroFGK107ZWaveMessage::setValue(const SensorData&, const uint8_t&)
{
}

void FibaroFGK107ZWaveMessage::setAfterStart()
{
}

int FibaroFGK107ZWaveMessage::getDeviceID()
{
	return DEVICE_ID_FIBARO_FGK_107;
}
