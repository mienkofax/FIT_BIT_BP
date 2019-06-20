#pragma once

#include <vector>
#include <string>

#include <Poco/Logger.h>

#include <Manager.h>

#include "model/SensorData.h"
#include "util/Loggable.h"

namespace BeeeOn {

#define COMMAND_CLASS_ALARM              113
#define COMMAND_CLASS_BATTERY            128
#define COMMAND_CLASS_BULGAR             10
#define COMMAND_CLASS_CONFIGURATION      112
#define COMMAND_CLASS_SENSOR_BINARY      48
#define COMMAND_CLASS_SENSOR_MULTILEVEL  49
#define COMMAND_CLASS_SWITCH_BINARY      37

#define SENSOR_INDEX_BULGAR              10
#define TEST_HOME_ID                     0xef1f4302

/*
 * Represents a BeeeOn sensor data sampled at a time.
 */
struct BeeeOnSensorValue {
	int moduleID;
	std::string value;
};

/*
 * Represents a single value from the Z-Wave network.
 */
struct ZWaveSensorValue {
	int commandClass;
	int index;
	OpenZWave::ValueID valueID;
	std::string value;
	std::string unit;
};

/*
 * Reports from products containing methods for processing
 * values from the network or to adjust values.
 */
class ZWaveMessage : public Loggable {
public:
	/*
	 * Extract data from ZWaveSensorValue struct and parse to BeeeOnSensorValue.
	 * It convert from command class and index to module id.
	 * @param &zwaveValues Values from Z-Wave network
	 * @return extracted data from ZWaveSensorValue
	 */
	virtual SensorData extractValues(
		const std::vector<ZWaveSensorValue> &zwaveValues) = 0;

	/*
	 * It sets data to Z-Wave device
	 * @param &beeeOnValues Data from adapter which sets for Z-Wave device
	 */
	virtual void setValue(const SensorData &sensorData,
		const uint8_t &nodeId) = 0;

	/*
	 * BeeeOn device ID
	 * @return device ID
	 */
	virtual int getDeviceID() = 0;

	/*
	 * Setting specific sensor data after start device
	 */
	virtual void setAfterStart() = 0;

	/*
	 * Create EUID for sensor.
	 * @param &homeId unique identifier ZWave network
	 * @param &nodeId unique identifier Zwave device
	 * @return 64bit euid
	 */
	uint64_t getEUID(const uint32_t &, const uint8_t &nodeId)
	{
		return ((int64(TEST_HOME_ID) << 8) | unsigned(nodeId));
	}

	virtual ~ZWaveMessage()
	{
	}

protected:
	bool asBool(const std::string &value);

	/*
	 * Check If equal.
	 * @param &commandClass1 info reports from ZWave network
	 * @param &commandClass2 info reports from ZWave network
	 * @param &index in command class info reports from ZWave network
	 * @param &index in command class info reports from ZWave network
	 * @return true if it equals
	 */
	bool isEqual(const int &commandClass1, const int &commandClass2,
		const int &index1, const int &index2) const;

	/*
	 * Parse float number from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value parsed message
	 * @param &item zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool extractFloat(double &value, const ZWaveSensorValue &item);
	/*
	 * Parse int number from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value parsed message
	 * @param &item zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool extractInt(int &value, const ZWaveSensorValue &item);

	/*
	 * Parse bool value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value parsed message
	 * @param &item zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool extractBool(bool &value, const ZWaveSensorValue &item);

	/*
	 * Parse string from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value parsed message
	 * @param &item zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool extractString(std::string &value, const ZWaveSensorValue &item);

	/*
	 * Parse specific sensor value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getSpecificValue(std::string &value, const int &commandClass, const int &index,
		const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Parse battery level value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getBatteryLevel(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Parse sensor value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getSensorValue(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Parse temperature value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getTemperature(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Parse switch binary value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getSwitchBinary(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);
	/*
	 * Parse luminance value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getLuminance(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Parse humidity value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getHumidity(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Parse ultraviolet value from ZWaveSensorValue by command class and index.
	 * ZWaveSensorValue contains value from ZWave network for specific device.
	 * @param &value sensor value
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &zwaveValues zwave sensor value vector
	 * @return true if it is was parsed successfully
	 */
	bool getUltraviolet(std::string &value, const std::vector<ZWaveSensorValue> &zwaveValues);

	/*
	 * Send data to ZWave network for setting ZWave device. It need to find out
	 * type of value which need to sent.
	 * @param &valueId it provides a unique ID for a value reported by a ZWave device
	 * @param &value it is value which contains data to setting
	 */
	void sendActuatorValue(const OpenZWave::ValueID &valueId, const std::string &value);

	/*
	 * Find value ID to be sets and send to ZWave netwrok.
	 * @param &value it is value to be sets
	 * @param &commandClass info reports from ZWave network
	 * @param &index index in command class info reports from ZWave network
	 * @param &nodeId unique identifier for device in ZWave network
	 */
	bool setActuator(const std::string &value, const int &commandClass, const int &index,
		const uint8_t &nodeId);
};

}
