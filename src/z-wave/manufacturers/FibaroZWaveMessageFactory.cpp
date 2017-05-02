#include <Poco/Exception.h>

#include "z-wave/manufacturers/FibaroZWaveMessageFactory.h"
#include "z-wave/products/FibaroFGK107ZWaveMessage.h"

using namespace BeeeOn;
using std::to_string;

ZWaveMessage *FibaroZWaveMessageFactory::create(uint32_t, uint32_t product)
{
	switch(product) {
	case FIBARO_FGK_107:
		return new FibaroFGK107ZWaveMessage();
	default:
		throw Poco::NotFoundException(
			"unsupported product " + to_string(product));
	}
}
