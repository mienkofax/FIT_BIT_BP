#include <Poco/Exception.h>

#include "z-wave/manufacturers/AeotecZWaveMessageFactory.h"

using namespace BeeeOn;

ZWaveMessage *AeotecZWaveMessageFactory::create(uint32_t manufacturer,
	uint32_t product)
{
	switch (product) {
	case AEOTEC_ZW100:
		return new AeotecZW100ZWaveMessage();
	default:
		throw Poco::NotFoundException("Unsupported product " + std::to_string(product));
	}
}
