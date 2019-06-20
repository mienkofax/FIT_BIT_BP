#pragma once

#include "z-wave/products/Popp123601ZWaveMessage.h"
#include "z-wave/ZWaveMessageFactory.h"

namespace BeeeOn {

#define POPP_MANUFACTURER       0x0154

/*
 * It creates Popp product.
 */
class PoppZWaveMessageFactory : public ZWaveMessageFactory {
public:
	ZWaveMessage *create(uint32_t manufacturer,
		uint32_t product) override;
};

}
