#include "zmq/ZMQDeviceManagerTable.h"

const uint8_t FIRST_DAEMON_IDENT = 0;

using namespace BeeeOn;
using namespace Poco;
using namespace std;

DeviceManagerID ZMQDeviceManagerTable::registerDaemonPrefix(
	const DevicePrefix &prefix)
{
	Mutex::ScopedLock guard(m_mutex);

	DeviceManagerID deviceManagerID = fromPrefix(prefix);
	m_deviceManagers.push_back(deviceManagerID);

	return deviceManagerID;
}

void ZMQDeviceManagerTable::unregisterDaemon(
	const DeviceManagerID &deviceManagerID)
{
	auto item = find(m_deviceManagers.begin(), m_deviceManagers.end(), deviceManagerID);

	if (item == m_deviceManagers.end())
		throw InvalidArgumentException(
			"device manager ID : " + deviceManagerID.toString()
			+ "not found");

	m_deviceManagers.erase(item);
}

DeviceManagerID ZMQDeviceManagerTable::fromPrefix(
	const DevicePrefix &prefix)
{
	if (m_deviceManagers.size() > UINT8_MAX)
		throw RangeException("maximum registered of device managers IDs");

	return DeviceManagerID(prefix, getFirstEmptyID());
}

uint8_t ZMQDeviceManagerTable::getFirstEmptyID()
{
	if (m_deviceManagers.empty())
		return FIRST_DAEMON_IDENT;

	uint8_t daemonID = m_deviceManagers.front().ident();

	for (auto &item : m_deviceManagers) {
		if (item.ident() == daemonID) {
			daemonID++;
			continue;
		}
		else {
			break;
		}
	}

	return daemonID;
}

std::vector<DeviceManagerID> ZMQDeviceManagerTable::getAll() const
{
	return m_deviceManagers;
}

std::vector<DeviceManagerID> ZMQDeviceManagerTable::getAll(
	const DevicePrefix &prefix) const
{
	std::vector<DeviceManagerID> deviceManagers;

	for (auto item : m_deviceManagers) {
		if (item.prefix() == prefix)
			deviceManagers.push_back(item);
	}

	return deviceManagers;
}

unsigned long ZMQDeviceManagerTable::count() const
{
	return m_deviceManagers.size();
}

bool ZMQDeviceManagerTable::isDeviceManagerRegistered(const DevicePrefix &prefix)
{
	Mutex::ScopedLock guard(m_mutex);

	for (auto item : m_deviceManagers) {
		if (prefix == item.prefix())
			return true;
	}

	return false;
}
