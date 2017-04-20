#include "zmq/ZMQMessageValueType.h"

using namespace BeeeOn;

EnumHelper<ZMQMessageValueTypeEnum::Raw>::ValueMap
	&ZMQMessageValueTypeEnum::valueMap()
{
	static EnumHelper<ZMQMessageValueTypeEnum::Raw>::ValueMap valueMap = {
		{ZMQMessageValueTypeEnum::TYPE_DOUBLE, "double"},
	};

	return valueMap;
}
