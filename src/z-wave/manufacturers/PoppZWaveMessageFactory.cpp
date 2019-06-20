#include <Poco/Exception.h>

#include "z-wave/products/Popp123601ZWaveMessage.h"
#include "z-wave/manufacturers/PoppZWaveMessageFactory.h"

using namespace BeeeOn;

ZWaveMessage *PoppZWaveMessageFactory::create(uint32_t, uint32_t product)
{
	switch(product) {
	case POPP_123601:
		return new Popp123601ZWaveMessage();
	default:
		throw Poco::NotFoundException("Unsupported product " + std::to_string(product));
	}
}
