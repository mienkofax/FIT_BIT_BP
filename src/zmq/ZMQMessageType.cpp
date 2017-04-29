#include "zmq/ZMQMessageType.h"

using namespace BeeeOn;

EnumHelper<ZMQMessageTypeEnum::Raw>::ValueMap &ZMQMessageTypeEnum::valueMap()
{
	static EnumHelper<ZMQMessageTypeEnum::Raw>::ValueMap valueMap = {
		{ZMQMessageTypeEnum::TYPE_ERROR, "error"},
		{ZMQMessageTypeEnum::TYPE_DEFAULT_RESULT, "default_result"},
		{ZMQMessageTypeEnum::TYPE_DEVICE_LAST_VALUE_CMD, "device_last_value_cmd"},
		{ZMQMessageTypeEnum::TYPE_DEVICE_LAST_VALUE_RESULT, "device_last_value_result"},
		{ZMQMessageTypeEnum::TYPE_DEVICE_LIST_CMD, "device_list_cmd"},
		{ZMQMessageTypeEnum::TYPE_DEVICE_LIST_RESULT, "device_list_result"},
		{ZMQMessageTypeEnum::TYPE_DEVICE_UNPAIR_CMD, "device_unpair_cmd"},
		{ZMQMessageTypeEnum::TYPE_HELLO_RESPONSE, "hello_response"},
		{ZMQMessageTypeEnum::TYPE_HELLO_REQUEST, "hello_request"},
		{ZMQMessageTypeEnum::TYPE_LISTEN_CMD, "listen_cmd"},
		{ZMQMessageTypeEnum::TYPE_MEASURED_VALUES, "measured_values"},
		{ZMQMessageTypeEnum::TYPE_SET_VALUES_CMD, "set_values_cmd"},
	};

	return valueMap;
}
