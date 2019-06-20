#include <string>
#include <vector>
#include <Poco/NumberParser.h>

#include "z-wave/products/Popp123601ZWaveMessage.h"

#define MODULE_SWITCH  0

using std::string;
using std::vector;

using namespace BeeeOn;

SensorData Popp123601ZWaveMessage::extractValues(
	const vector<ZWaveSensorValue> &zwaveValues)
{
	SensorData sensorData;
	string data;

	if (getSwitchBinary(data, zwaveValues))
		sensorData.insertValue(SensorValue(
			ModuleID(MODULE_SWITCH),
			Poco::NumberParser::parseFloat(data)));

	return sensorData;
}

void Popp123601ZWaveMessage::setValue(const SensorData &sensorData,
	const uint8_t &nodeId)
{
	for (auto data : sensorData) {
		if (data.moduleID().value() == 0)
			setActuator(std::to_string(data.value()), COMMAND_CLASS_SWITCH_BINARY, 0, nodeId);
	}
}

void Popp123601ZWaveMessage::setAfterStart()
{
}

int Popp123601ZWaveMessage::getDeviceID()
{
	return DEVICE_ID_POPP_123601;
}
