#ifndef GATEWAY_JABLOTRON_H
#define GATEWAY_JABLOTRON_H

#include <Poco/StringTokenizer.h>
#include <Poco/Timer.h>

#include "core/DeviceManager.h"
#include "jablotron/SerialControl.h"
#include "model/DeviceID.h"
#include "model/SensorData.h"
#include "zmq/ZMQClient.h"

namespace BeeeOn {

class JablotronDeviceManager : public DeviceManager {
public:
	JablotronDeviceManager();

	void run() override;

	void setDonglePath(const std::string &path);

private:
	typedef uint32_t JablotronSerialNumber;

	struct AC88_ModuleID {
		enum {
			SENSOR_STATE = 0,
		};
	};

	struct JA85ST_ModuleID {
		enum {
			FIRE_SENSOR = 0,
			FIRE_SENSOR_ALARM,
			FIRE_SENSOR_ERROR,
			BATTERY_STATE,
		};
	};

	struct JA83P_JA82SH_ModuleID {
		enum {
			SENSOR = 0,
			SENSOR_ALARM,
			BATTERY_STATE,
		};
	};

	struct JA81M_JA83M_ModuleID {
		enum {
			DOOR_CONTACT = 0,
			DOOR_CONTACT_ALARM,
			BATTERY_STATE,
		};
	};

	struct RC86K_ModuleID {
		enum {
			REMOTE_CONTROL = 0,
			REMOTE_CONTROL_ALARM,
			BATTERY_STATE,
		};
	};

	struct TP82N_ModuleID {
		enum {
			CURRENT_ROOM_TEMPERATURE = 0,
			REQUESTD_ROOM_TEMPERATURE,
			BATTERY_STATE,
		};
	};

	struct SENSOR_VALUE {
		enum {
			UNKNOWN_VALUE = 0,
			STATE_ON,
			STATE_OFF,
		};
	};

	struct FIRE_SENSOR_VALUE {
		enum {
			STATE_OFF = 0,
			STATE_ON,
		};
	};

	enum DeviceList {
		AC88 = 8,
		JA80L = 9,
		TP82N = 10,
		JA83M = 11,
		JA81M = 18,
		JA82SH = 19,
		JA83P = 20,
		JA85ST = 21,
		RC86K = 22,
	};

	void onEvent(const void*, ZMQMessage &zmqMessage) override;

	void initJablotronSerial();

	void checkQueue();

	void getDeviceList();
	void getLastValue(const DeviceID &deviceID);

	void startListen(Poco::Timer &timer);
	void stopListen(Poco::Timer &timer);

	void doDeviceLastValueResult(ZMQMessage &zmqMessage);
	void doTypeDeviceListResult(ZMQMessage &zmqMessage);
	void doListenCommand(ZMQMessage &zmqMessage);
	void doDeviceUnpairCommand(ZMQMessage &zmqMessage);
	void doSetValuesCommand(ZMQMessage &zmqMessage);

	/*
	 * True ak nacitana sprava validna inak false.
	 */
	bool readFromSerial(std::string &data);

	/*
	 * @brief It sends data from devices to server
	 * @param &jablotron_msg Device identifiers to set
	 * @param &token String contain the value
	 * @return if value has valid data
	 */
	void sendMeasuredValues(const std::string &message,
		Poco::StringTokenizer& token);

	/*
	 * @brief It locates the serial number of device
	 * @param euid Device EUID
	 * @return Serial number of device
	 */
	JablotronSerialNumber getSerialNumber(const std::string &token);

	/*
	 * Parse a value which was read from usb dongle and send to server
	 * @param & jablotron_msg Struct contains jablotron data
	 * @param &token String will be diveded according spaces
	 *
	 */
	SensorData parseMessageFromDevice(const std::string& message,
		Poco::StringTokenizer &token);

	/*
	 * It sets a value which was read from TP-82N
	 * @param &serialMessage String contain the substring which was read from usb dongle
	 * @param &data String contains a temperature
	 * which can appear outdoor or indoor
	 */
	SensorValue parseMessageFromTP82N(const std::string &message,
		const std::string &data);

	/*
	 * It sets a value which was read from RC-86K
	 * @param &data String contains the values from the sensor
	 */
	SensorValue parseMessageFromRC86K(const std::string &data);

	/*
	 * It sets a value which was read from JA-81M and JA-83M
	 * @param &data String contains the values from the sensor
	 */
	SensorValue parseMessageFromJA81M_JA83M(const Poco::StringTokenizer &token);

	/*
	 * It sets a value which was read from JA-83P and JA-82SH
	 * @param &token String contains the values from the sensor
	 */
	SensorValue parseMessageFromJA83P_JA82SH(const Poco::StringTokenizer &token);

	/*
	 * It sets a value which was read from JA-85ST
	 * @param &token String contains the values from the sensor
	 */
	SensorValue parseMessageFromJA85ST(const Poco::StringTokenizer &token);

	/*
	 * @brief It converts string value to float
	 * @param &msg String contains a float value
	 * @return Float sensor value
	 */
	double getValue(const std::string& msg) const;

	/*
	 * @brief Convert jablotron state to BeeOn state
	 * @param &data String contains state value from sensor
	 * @param reverse If true/false state is replaced
	 * @return Float state value
	 */
	double convert(const std::string& data, bool reverse = true) const;

	/*
	 * @brief Convert Jablotron battery state to BeeOn value
	 * for example 0 => 0%, 1=>100%
	 * @param String value of battery
	 * @return Battery value
	 */
	int getBatteryStatus(const std::string& msg) const;

	/*
	 * @brief Loading of registered devices from the Turris Dongle
	 * @return if it has been loaded successfully
	 */
	bool loadRegDevices();

	/*
	 * @brief It checks if the Turris Dongle is connected.
	 * @param &loadDevices If devices has been loaded successfully or unsuccessful
	 * @return If it has been loaded succesfully and connected or if
	 * it could not load the device
	 */
	bool loadJablotronDevices(bool &loadDevices);

	/*
	 * @brief It locates the device ID
	 * @param sn The serial number of device
	 * @return Device ID
	 */
	int getDeviceType(JablotronSerialNumber serialNumber) const;

	/*
	 * @brief It converts serial number of device to Device EUID
	 * @param sn The serial number of device
	 * @return DeviceID
	 */
	DeviceID createDeviceID(JablotronSerialNumber serialNumber) const;

	/*
	 * @brief It sendings data to the Turris Dongle according
	 * to the Jablotron protocol
	 * @param &msg Sending data
	 */
	void retransmissionPacket(const std::string &msg);

	/*
	 * @brief It sets the value of the switch
	 * @param euid Device EUID
	 * @param sw Switch value
	 */
	void setSwitch(const DeviceID &deviceID, short sw);

private:
	std::string m_donglePath;
	Poco::SharedPtr<SerialControl> m_serial;
	std::vector<JablotronSerialNumber> m_devices;
	bool m_sensorEvent;
	SensorValue m_sensorEventValue;
	AnswerQueue m_queue;

	Poco::AtomicCounter m_queueLoop;
	std::set<DeviceID> m_devicesWithFlag;
	Poco::TimerCallback<JablotronDeviceManager> m_callback;
	Poco::AtomicCounter m_listen;
	Poco::Timer m_deferAfter;
	std::string m_buffer;

	/*
	 * Field X - output X
	 * Slot which contains the state of the first AC-88 device
	 */
	short pgx = 0;

	/*
	 * Field Y - output Y
	 * Slot which contains the state of the second AC-88 device
	 */
	short pgy = 0;
};

}

#endif
