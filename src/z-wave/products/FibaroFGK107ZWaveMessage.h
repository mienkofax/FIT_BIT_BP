#pragma once

#include "z-wave/ZWaveMessage.h"

namespace BeeeOn {

#define FIBARO_FGK_107              0x1000
#define DEVICE_ID_FIBARO_FGK_107    25

class FibaroFGK107ZWaveMessage : public ZWaveMessage {
public:
	SensorData extractValues(
		const std::vector<ZWaveSensorValue> &zwaveValues) override;

	void setValue(const SensorData &sensorData,
		const uint8_t &nodeId) override;

	void setAfterStart() override;

	int getDeviceID() override;
};

}
