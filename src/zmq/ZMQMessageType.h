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
 *
 * 3. message_type: hello_response
 *
 * Sprava obsahujuca vygenerovane DeviceManagerID, ktore sa dalej
 * pouzije pre identifikaciu datoveho socketu.
 *
 * {
 *     "message_type" : "hello_response",
 *     "device_manager_id" : "0xa100"
 * }
 *
 * 4. message_type: measured_values
 *
 * Psrava s nameranymi hodnota zo zariadenia. Pre identifikaciu
 * zariadenia, s ktoreho bola odoslana sa pouziva device_id.
 * Jednotlive hodnoty su reprezentovane modulom, ktory nameral hodnotu
 * nameranou hodnotou a typom nameranej hodnoty.
 *
 * {
 *     "message_type" : "measured_values",
 *     "device_id" : "0x132465789",
 *     "values" : [
 *         {
 *             "raw" : "103.5",
 *             "type" : "double",
 *             "module_id" : "0"
 *         }
 *     ]
 * }
 *
 * 5. message_type: listen_cmd
 *
 * Sprava po ktorej sa zapne parovaci rezim. Parovaci rezim je urceny
 * pre prijimanie sprav od novych zariadeni.
 *
 * {
 *     "message_type" : "listen_cmd",
 *     "duration" : 60
 * }
 */
struct ZMQMessageTypeEnum {
	enum Raw {
		TYPE_ERROR,
		TYPE_HELLO_RESPONSE,
		TYPE_HELLO_REQUEST,
		TYPE_LISTEN_CMD,
		TYPE_MEASURED_VALUES,
	};

	static EnumHelper<Raw>::ValueMap &valueMap();
};

typedef Enum<ZMQMessageTypeEnum> ZMQMessageType;

}

#endif
