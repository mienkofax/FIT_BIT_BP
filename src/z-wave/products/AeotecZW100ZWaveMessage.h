#pragma once

#include <map>

#include "z-wave/ZWaveMessage.h"

namespace BeeeOn {

#define AEOTEC_ZW100              0x0064
#define DEVICE_ID_AEOTEC_ZW100    27

class AeotecZW100ZWaveMessage : public ZWaveMessage {
public:
	AeotecZW100ZWaveMessage();

	SensorData extractValues(
		const std::vector<ZWaveSensorValue> &zwaveValues) override;

	void setValue(const SensorData &sensorData,
		const uint8_t &nodeId) override;

	void setAfterStart() override;

	int getDeviceID() override;

private:
	std::map<std::string, std::string> m_pirSensor;

	void setMapValue();

	bool getPirSensitivity(std::string& value,
		const std::vector<ZWaveSensorValue> &zwaveValues);

	bool getRefreshTime(std::string& value,
		const std::vector<ZWaveSensorValue> &zwaveValues);

	void getDetectionSensor(SensorData &sensorData,
		const std::vector<ZWaveSensorValue> &zwaveValues);
};

}
