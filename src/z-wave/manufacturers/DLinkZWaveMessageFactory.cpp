#include <Poco/Exception.h>

#include "z-wave/manufacturers/DLinkZWaveMessageFactory.h"

using namespace BeeeOn;

ZWaveMessage *DLinkZWaveMessageFactory::create(uint32_t, uint32_t product)
{
	switch (product) {
	case DLINK_DCH_Z120: {
		return new DLinkDchZ120ZWaveMessage();
	}
	default:
		throw Poco::NotFoundException("Unsupported product");
	}
}
