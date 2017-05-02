#include <string>

#include "z-wave/GenericZWaveMessageFactory.h"

using namespace BeeeOn;
using std::to_string;

ZWaveMessage *GenericZWaveMessageFactory::create(uint32_t manufacturer,
	uint32_t product)
{
	auto search = m_manufacturers.find(manufacturer);

	if (search != m_manufacturers.end())
		return search->second->create(manufacturer, product);

	throw Poco::Exception("Manufacturer " + to_string(manufacturer)
		+ " is not registered");
}

void GenericZWaveMessageFactory::registerManufacturer(uint32_t manufacturer,
		Poco::SharedPtr<ZWaveMessageFactory> factory)
{
	if (factory.isNull())
		throw Poco::NullPointerException("null pointer to Z-Wave factory");

	m_manufacturers.insert(std::make_pair(manufacturer, factory));
}
