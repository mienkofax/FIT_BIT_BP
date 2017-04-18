#ifndef BEEEON_ZMQ_DEVICE_MANAGER_TABLE_H
#define BEEEON_ZMQ_DEVICE_MANAGER_TABLE_H

#include <vector>

#include <Poco/Mutex.h>

#include "model/DeviceManagerID.h"
#include "model/DevicePrefix.h"

namespace BeeeOn {

/*
 * List of device managers joined (connected) to zmq-broker
 * using data socket. Before communication can start using
 * this socket, device asks about ID that will be used during
 * the communication. The request contains prefix that marks
 * the type of device manager.
 */
class ZMQDeviceManagerTable {
public:
	DeviceManagerID registerDaemonPrefix(const DevicePrefix &prefix);
	void unregisterDaemon(const DeviceManagerID &deviceManagerID);

	std::vector<DeviceManagerID> getAll() const;
	std::vector<DeviceManagerID> getAll(
		const DevicePrefix &prefix) const;

	unsigned long count() const;

	bool isDeviceManagerRegistered(const DevicePrefix &prefix);

private:
	uint8_t getFirstEmptyID();

	/*
	 * Create new device manager id from prefix for communication.
	 */
	DeviceManagerID fromPrefix(const DevicePrefix &prefix);

private:
	std::vector<DeviceManagerID> m_deviceManagers;
	Poco::Mutex m_mutex;
};

}

#endif
