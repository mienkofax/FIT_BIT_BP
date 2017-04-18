#ifndef BEEEON_ZMQ_PROTOCOL_MESSAGE_H
#define BEEEON_ZMQ_PROTOCOL_MESSAGE_H

#include <string>

#include <Poco/JSON/Object.h>

#include "zmq/ZMQMessageType.h"

namespace BeeeOn {

/*
 * The class represents messages which send or receive using zmq.
 *
 * Umoznuje vytvarat spravy z pozadovanych objektov a naopak.
 * Metody zacinajuce toXXX() vytvaraju objekty z prijatej json spravy.
 * Pred volanim tychto metod je nutne vyvorit ZMQMessage pomocou
 * ZMQMessage::fromJSON() aby sa rozparsovala prijata JSON sprava.
 * Pre vytvaranie sprav z objektov su pripravene metody zacinajuce
 * fromXXX(), ktore vyvoria ZMQMessage. Vytvorena sprava sa prevedie
 * na retazec pomocout toString().
 *
 * Metody zacinajuce setXXX() nastavuju urcity parameter do json spravy
 * a metody zacinajuce getXXX() vyberaju pozadovane data z json spravy.
 */
class ZMQMessage {
public:
	/*
	 * Creates empty message.
	 */
	ZMQMessage();

	/*
	 * Type of message.
	 */
	ZMQMessageType type() const;

	std::string toString() const;

	/*
	 * Parses json message and store into Poco::JSON::Object (m_json).
	 */
	static ZMQMessage fromJSON(const std::string &json);

private:
	/*
	 * Creates message from parsed json message.
	 */
	ZMQMessage(Poco::JSON::Object::Ptr jsonObject);

	/*
	 * {
	 *     "message_type" : "type"
	 * }
	 */
	void setType(const ZMQMessageType &type);

private:
	Poco::JSON::Object::Ptr m_json;
};

}

#endif
