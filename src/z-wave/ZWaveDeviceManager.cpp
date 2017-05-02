#include <Options.h>
#include <Manager.h>

#include "di/Injectable.h"
#include "z-wave/ZWaveDeviceManager.h"

BEEEON_OBJECT_BEGIN(BeeeOn, ZWaveDeviceManager)
BEEEON_OBJECT_CASTABLE(StoppableRunnable)
BEEEON_OBJECT_TEXT("dataServerHost", &ZWaveDeviceManager::setDataServerHost)
BEEEON_OBJECT_NUMBER("dataServerPort", &ZWaveDeviceManager::setDataServerPort)
BEEEON_OBJECT_TEXT("helloServerHost", &ZWaveDeviceManager::setHelloServerHost)
BEEEON_OBJECT_NUMBER("helloServerPort", &ZWaveDeviceManager::setHelloServerPort)
BEEEON_OBJECT_TEXT("prefixName", &ZWaveDeviceManager::setPrefixName)
BEEEON_OBJECT_TEXT("setUserPath", &ZWaveDeviceManager::setUserPath)
BEEEON_OBJECT_TEXT("donglePath", &ZWaveDeviceManager::setDonglePath)
BEEEON_OBJECT_TEXT("setConfigPath", &ZWaveDeviceManager::setConfigPath)
BEEEON_OBJECT_NUMBER("setPollInterval", &ZWaveDeviceManager::setPollInterval)
BEEEON_OBJECT_NUMBER("setDriverMaxAttempts", &ZWaveDeviceManager::setDriverMaxAttempts)
BEEEON_OBJECT_NUMBER("setSaveConfigurationFile", &ZWaveDeviceManager::setSaveConfigurationFile)

BEEEON_OBJECT_END(BeeeOn, ZWaveDeviceManager)

using namespace BeeeOn;
using namespace OpenZWave;

void ZWaveDeviceManager::onEvent(const void *, ZMQMessage &zmqMessage)
{

}

void ZWaveDeviceManager::setUserPath(const std::string &userPath)
{
	m_userPath = userPath;
}

void ZWaveDeviceManager::setDonglePath(const std::string &donglePath)
{
	m_donglePath = donglePath;
}

void ZWaveDeviceManager::setConfigPath(const std::string &configPath)
{
	m_configPath = configPath;
}

void ZWaveDeviceManager::setPollInterval(int pollInterval)
{
	m_pollInterval = pollInterval;
}

void ZWaveDeviceManager::setDriverMaxAttempts(int maxAttempts)
{
	m_driverMaxAttempts = maxAttempts;
}

void ZWaveDeviceManager::setSaveConfigurationFile(bool save)
{
	m_saveConfigurationFile = save;
}

void ZWaveDeviceManager::installOption()
{
	OpenZWave::Options::Create(m_configPath, m_userPath, "");
	Options::Get()->AddOptionInt("PollInterval", m_pollInterval);
	Options::Get()->AddOptionBool("logging", true);
	Options::Get()->AddOptionBool("SaveConfiguration", m_saveConfigurationFile);
	Options::Get()->AddOptionInt("DriverMaxAttempts", m_driverMaxAttempts);
	Options::Get()->Lock();
}

void ZWaveDeviceManager::run()
{
	installOption();
	DeviceManager::runClient();

	m_driver.assign(new ZWaveDriver(m_donglePath));
	Manager::Create();
	m_driver->registerItself();
}

void ZWaveDeviceManager::stop()
{
	m_driver->unregisterItself();

	Manager::Destroy();
	Options::Destroy();

	DeviceManager::stop();
}
