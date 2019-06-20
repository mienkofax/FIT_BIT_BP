#include <Poco/Exception.h>

#include "z-wave/manufacturers/PhilioZWaveMessageFactory.h"

using namespace BeeeOn;

ZWaveMessage *PhilioZWaveMessageFactory::create(uint32_t, uint32_t product)
{
	switch (product) {
	case PHILIO_PST02_1C: {
		return new PhilioPST021CZWaveMessage();
	}
	default:
		throw Poco::NotFoundException("Unsupported product");
	}
}
