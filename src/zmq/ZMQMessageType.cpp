#include "zmq/ZMQMessageType.h"

using namespace BeeeOn;

EnumHelper<ZMQMessageTypeEnum::Raw>::ValueMap &ZMQMessageTypeEnum::valueMap()
{
	static EnumHelper<ZMQMessageTypeEnum::Raw>::ValueMap valueMap = {
		{ZMQMessageTypeEnum::TYPE_ERROR, "error"},
	};

	return valueMap;
}
