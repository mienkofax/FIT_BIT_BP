#ifndef BEEEON_ZWAVE_DEVICE_MANAGER_H
#define BEEEON_ZWAVE_DEVICE_MANAGER_H

#include <string.h>
#include <Poco/Timer.h>

#include "core/AnswerQueue.h"
#include "core/DeviceManager.h"
#include "z-wave/NotificationProcessor.h"
#include "z-wave/ZWaveDriver.h"

namespace BeeeOn {

class ZWaveDeviceManager : public DeviceManager {
public:
	ZWaveDeviceManager();

	void run() override;
	void stop() override;

	void setUserPath(const std::string &userPath);
	void setDonglePath(const std::string &donglePath);
	void setConfigPath(const std::string &configPath);
	void setPollInterval(int pollInterval);
	void setDriverMaxAttempts(int maxAttempts);
	void setSaveConfigurationFile(bool save);

protected:
	void onEvent(const void*, ZMQMessage &zmqMessage) override;

	void installOption();
	void installManufacturers();

	void getDeviceList();
	void getLastValue(const DeviceID &deviceID, const ModuleID &moduleID);
	void checkQueue();

	void startListen();
	void stopListen(Poco::Timer &timer);
	void stopUnpair(Poco::Timer &timer);

	void setLastState();

	void doDeviceListResult(ZMQMessage &zmqMessage);
	void doListenCommand(ZMQMessage &zmqMessage);
	void doDeviceLastValueResult(ZMQMessage &zmqMessage);
	void doDeviceUnpairCommand(ZMQMessage &zmqMessage);
	void doSetValueCommand(ZMQMessage &zmqMessage);

protected:
	std::string m_userPath;
	std::string m_donglePath;
	std::string m_configPath;
	int m_pollInterval;
	int m_driverMaxAttempts;
	bool m_saveConfigurationFile;
	uint32_t m_homeId;
	Poco::SharedPtr<ZWaveDriver> m_driver;
	NotificationProcessor m_notificationProcessor;
	GenericZWaveMessageFactory m_factory;

	std::set<DeviceID> m_devices;
	AnswerQueue m_queue;
	Poco::AtomicCounter m_listen;
	Poco::TimerCallback<ZWaveDeviceManager> m_callback;
	Poco::Timer m_derefListen;

	Poco::TimerCallback<ZWaveDeviceManager> m_callbackUnpair;
	Poco::Timer m_derefUnpair;
};

}

#endif
