#pragma once

#include "z-wave/ZWaveMessage.h"

namespace BeeeOn {

#define POPP_123601              0x0001
#define DEVICE_ID_POPP_123601    17

class Popp123601ZWaveMessage : public ZWaveMessage {
public:
	SensorData extractValues(
		const std::vector<ZWaveSensorValue> &zwaveValues) override;

	void setValue(const SensorData &sensorData,
		const uint8_t &nodeId) override;

	void setAfterStart() override;

	int getDeviceID() override;
};

}
