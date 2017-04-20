#ifndef BEEEON_ZMQ_PROTOCOL_MESSAGE_VALUE_TYPE_H
#define BEEEON_ZMQ_PROTOCOL_MESSAGE_VALUE_TYPE_H

#include "util/Enum.h"

namespace BeeeOn {

/*
 * It is a representation of the basic data types in JSON
 * attribute called "values".
 *
 * The simple usage in the JSON protocol:
 * "values" : [
 *   {
 *     "type" : "double",
 *     "raw" : "123.01"
 *   }
 * ]
 */
struct ZMQMessageValueTypeEnum {
	enum Raw {
		TYPE_DOUBLE,
	};

	static EnumHelper<Raw>::ValueMap &valueMap();
};

typedef Enum<ZMQMessageValueTypeEnum> ZMQMessageValueType;

}

#endif
