#pragma once

#include "z-wave/ZWaveMessageFactory.h"
#include "z-wave/products/DLinkDchZ120ZWaveMessage.h"

namespace BeeeOn {

#define DLINK_MANUFACTURER      0x0108

/*
 * It creates D-Link product.
 */
class DLinkZWaveMessageFactory : public ZWaveMessageFactory {
public:
	ZWaveMessage *create(uint32_t manufacturer, uint32_t product) override;
};

}
