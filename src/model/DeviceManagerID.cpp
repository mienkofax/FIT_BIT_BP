#include <climits>

#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>

#include "model/DeviceManagerID.h"

using namespace BeeeOn;
using namespace Poco;
using namespace std;

const int PREFIX_SHIFT = 8;

DeviceManagerID::DeviceManagerID():
	m_id(0)
{
}

DeviceManagerID::DeviceManagerID(uint16_t id):
	m_id(id)
{
	DevicePrefix prefix = DevicePrefix::fromRaw(m_id >> PREFIX_SHIFT);

	if (prefix == DevicePrefix::PREFIX_INVALID)
		throw InvalidArgumentException(
			"no prefix present in the given device manager ID: "
			+ to_string(m_id));
}

DeviceManagerID::DeviceManagerID(const DevicePrefix &prefix, uint8_t ident)
{
	m_id = ((uint8_t) prefix.raw()) << PREFIX_SHIFT;
	m_id |= ident;
}

DeviceManagerID DeviceManagerID::parse(const string &s)
{
	unsigned deviceManagerID;

	deviceManagerID = Poco::NumberParser::parseHex(s);
	if (deviceManagerID > USHRT_MAX)
		throw RangeException(
			"device manager ID too high: " + to_string(deviceManagerID));

	return DeviceManagerID((uint16_t) deviceManagerID);
}

std::string DeviceManagerID::toString() const
{
	return Poco::NumberFormatter::formatHex(m_id);
}

DevicePrefix DeviceManagerID::prefix() const
{
	return DevicePrefix::fromRaw(m_id >> PREFIX_SHIFT);
}
