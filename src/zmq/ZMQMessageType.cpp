#include "zmq/ZMQMessageType.h"

using namespace BeeeOn;

EnumHelper<ZMQMessageTypeEnum::Raw>::ValueMap &ZMQMessageTypeEnum::valueMap()
{
	static EnumHelper<ZMQMessageTypeEnum::Raw>::ValueMap valueMap = {
		{ZMQMessageTypeEnum::TYPE_ERROR, "error"},
		{ZMQMessageTypeEnum::TYPE_HELLO_RESPONSE, "hello_response"},
		{ZMQMessageTypeEnum::TYPE_HELLO_REQUEST, "hello_request"},
		{ZMQMessageTypeEnum::TYPE_LISTEN_CMD, "listen_cmd"},
		{ZMQMessageTypeEnum::TYPE_MEASURED_VALUES, "measured_values"},
	};

	return valueMap;
}
