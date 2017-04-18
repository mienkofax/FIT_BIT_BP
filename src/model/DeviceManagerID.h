#ifndef BEEEON_DEVICE_MANAGER_ID_H
#define BEEEON_DEVICE_MANAGER_ID_H

#include <string>

#include "model/DevicePrefix.h"

namespace BeeeOn {

/*
 * DeviceManagerID is a unique identification between zmq broker
 * and DeviceManagers. It contains two parts. First part im_ids
 * DevicePrefix which identifies type of DeviceManagers
 * (Z-Wave, Jablotron, ...) and unique identifier.
 */
class DeviceManagerID {
public:
	DeviceManagerID();

	DeviceManagerID(uint16_t id);

	DeviceManagerID(const DevicePrefix &prefix, uint8_t ident);

	uint16_t value() const
	{
		return m_id;
	}

	uint8_t ident() const
	{
		return (uint8_t) m_id & (uint8_t) 0xff;
	}

	bool isValid() const
	{
		return m_id == 0;
	}

	DevicePrefix prefix() const;

	static DeviceManagerID parse(const std::string &s);

	std::string toString() const;

	bool operator !=(const DeviceManagerID &id) const
	{
		return m_id != id.m_id;
	}

	bool operator ==(const DeviceManagerID &id) const
	{
		return m_id == id.m_id;
	}

	bool operator <(const DeviceManagerID &id) const
	{
		return m_id < id.m_id;
	}

	bool operator >(const DeviceManagerID &id) const
	{
		return m_id > id.m_id;
	}

	bool operator <=(const DeviceManagerID &id) const
	{
		return m_id <= id.m_id;
	}

	bool operator >=(const DeviceManagerID &id) const
	{
		return m_id >= id.m_id;
	}

	operator unsigned short() const
	{
		return m_id;
	}

	operator uint16_t()
	{
		return m_id;
	}

private:
	uint16_t m_id;
};

}

#endif
