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
 *
 * 2. message_type: hello_request
 *
 * DeviceManager MUST be always registered with its prefix before it
 * starts to send or receive messages with commands (from device
 * managers).
 *
 * {
 *     "message_type" : "hello_request",
 *     "device_manager_prefix" : "Fitprotocol"
 * }
 */
struct ZMQMessageTypeEnum {
	enum Raw {
		TYPE_ERROR,
		TYPE_HELLO_REQUEST,
	};

	static EnumHelper<Raw>::ValueMap &valueMap();
};

typedef Enum<ZMQMessageTypeEnum> ZMQMessageType;

}

#endif
