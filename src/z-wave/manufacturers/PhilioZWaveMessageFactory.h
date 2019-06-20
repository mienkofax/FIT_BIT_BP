#pragma once

#include "z-wave/ZWaveMessageFactory.h"
#include "z-wave/products/PhilioPST021CZWaveMessage.h"

namespace BeeeOn {

#define PHILIO_MANUFACTURER      0x013c

/*
 * It creates Philio product.
 */
class PhilioZWaveMessageFactory : public ZWaveMessageFactory {
public:
	ZWaveMessage *create(uint32_t manufacturer, uint32_t product) override;
};

}
