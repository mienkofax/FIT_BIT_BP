#pragma once

#include "z-wave/ZWaveMessage.h"

namespace BeeeOn {

#define PHILIO_PST02_1C             0x000e
#define DEVICE_ID_PHILIO_PST02_1C   16

class PhilioPST021CZWaveMessage : public ZWaveMessage {
public:
	SensorData extractValues(
		const std::vector<ZWaveSensorValue> &zwaveValues) override;

	void setValue(const SensorData &sensorData,
		const uint8_t &nodeId) override;

	void setAfterStart() override;

	int getDeviceID() override;
};

}
