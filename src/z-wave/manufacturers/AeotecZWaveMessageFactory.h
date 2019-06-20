#pragma once

#include "z-wave/ZWaveMessageFactory.h"
#include "z-wave/products/AeotecZW100ZWaveMessage.h"

namespace BeeeOn {

#define AEOTEC_MANUFACTURER     0x0086

/*
 * It creates Aeotec products.
 */
class AeotecZWaveMessageFactory : public ZWaveMessageFactory {
public:
	ZWaveMessage *create(uint32_t manufacturer, uint32_t product) override;
};

}
