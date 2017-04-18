#ifndef BEEEON_ZMQ_MESSAGE_TYPE_H
#define BEEEON_ZMQ_MESSAGE_TYPE_H

#include "util/Enum.h"

namespace BeeeOn {

/*
 * The class represents all types of messages that may be sent/received
 * using zmq. It defines the following type of messages:
 *
 * 1. message_type: error
 *
 * It is a response to an unknown message/attribute/command.
 *
 * {
 *     "message_type" : "error",
 *     "error_code" : 0,
 *     "error_message" : "unknown error"
 * }
 */
struct ZMQMessageTypeEnum {
	enum Raw {
		TYPE_ERROR,
	};

	static EnumHelper<Raw>::ValueMap &valueMap();
};

typedef Enum<ZMQMessageTypeEnum> ZMQMessageType;

}

#endif
