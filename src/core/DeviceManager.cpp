#include "core/DeviceManager.h"
#include "util/ZMQUtil.h"

using namespace BeeeOn;

DeviceManager::DeviceManager():
	m_stop(false),
	m_prefix(DevicePrefix::fromRaw(DevicePrefix::PREFIX_INVALID)),
	m_zmqClient(new ZMQClient())
{
	m_runner.addRunnable(m_zmqClient);
}

void DeviceManager::stop()
{
	m_runner.stop();
	m_stop = true;
}

void DeviceManager::setPrefixName(const std::string &name)
{
	m_prefix = DevicePrefix::parse(name);
	m_zmqClient->setDeviceManagerPrefix(m_prefix);
}

void DeviceManager::setDataServerHost(const std::string &host)
{
	m_zmqClient->setDataServerHost(host);
}

void DeviceManager::setHelloServerHost(const std::string &host)
{
	m_zmqClient->setHelloServerHost(host);
}

void DeviceManager::setDataServerPort(const int port)
{
	m_zmqClient->setDataServerPort(port);
}

void DeviceManager::setHelloServerPort(const int port)
{
	m_zmqClient->setHelloServerPort(port);
}

void DeviceManager::runClient()
{
	m_runner.start();
}
