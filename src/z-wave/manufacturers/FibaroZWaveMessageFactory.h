#pragma once

#include "z-wave/ZWaveMessageFactory.h"

namespace BeeeOn {

#define FIBARO_MANUFACTURER       0x010f

/*
 * It creates Fibaro product.
 */
class FibaroZWaveMessageFactory : public ZWaveMessageFactory {
public:
	ZWaveMessage *create(uint32_t manufacturer, uint32_t product) override;
};

}
