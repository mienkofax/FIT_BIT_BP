#pragma once

#include "z-wave/ZWaveMessage.h"

namespace BeeeOn {

#define DLINK_DCH_Z120             0x000d
#define DEVICE_ID_DLINK_DCH_Z120   26

class DLinkDchZ120ZWaveMessage : public ZWaveMessage {
public:
	SensorData extractValues(
		const std::vector<ZWaveSensorValue> &zwaveValues) override;

	void setValue(const SensorData &sensorData,
		const uint8_t &nodeId) override;

	void setAfterStart() override;

	int getDeviceID() override;

private:
	bool getPirSensitivity(std::string &value,
		const std::vector<ZWaveSensorValue> &zwaveValues);

	void getDetectionSensor(SensorData &sensorData,
		const std::vector<ZWaveSensorValue> &zwaveValues);
};

}
