#pragma once

#include <map>

#include <Poco/SharedPtr.h>

#include "z-wave/ZWaveMessageFactory.h"

namespace BeeeOn {

class GenericZWaveMessageFactory : public ZWaveMessageFactory {
public:
	ZWaveMessage *create(uint32_t manufacturer,
		uint32_t product) override;

	/*
	 * Register Z-Wave manufacturer to factory using manufacturer id.
	 * @param &manufacturer Unique number of manufacturer
	 * @param &factory Manufacturer object
	 */
	void registerManufacturer(uint32_t manufacturer,
		Poco::SharedPtr<ZWaveMessageFactory> factory);

private:
	std::map<uint32_t, Poco::SharedPtr<ZWaveMessageFactory>> m_manufacturers;
};

}
