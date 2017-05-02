#ifndef BEEEON_ZWAVE_DEVICE_MANAGER_H
#define BEEEON_ZWAVE_DEVICE_MANAGER_H

#include <string.h>

#include "core/DeviceManager.h"
#include "z-wave/ZWaveDriver.h"

namespace BeeeOn {

class ZWaveDeviceManager : public DeviceManager {
public:
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

protected:
	std::string m_userPath;
	std::string m_donglePath;
	std::string m_configPath;
	int m_pollInterval;
	int m_driverMaxAttempts;
	bool m_saveConfigurationFile;
	Poco::SharedPtr<ZWaveDriver> m_driver;
};

}

#endif
