#ifndef BEEEON_ZMQ_UTIL_H
#define BEEEON_ZMQ_UTIL_H

#include <string>

#include <Poco/SharedPtr.h>

#include <zmq.hpp>

namespace BeeeOn {

/*
 * Zjednodusenie posielania a prijimania sprav pomocou ZMQ protokolu.
 * Metoda na prijem dat je neblokujuca. Ak sa jedna o endpoint typu
 * ZMQ_ROUTER (async server) a komunikuje sa s clientom, ktory ma
 * endpoint typu ZMQ_DEALER, tak pred samostatnym odoslanim dat pomocou
 * send je nutne poslat spravy s identifikatorom pomoocout sendMultipart.
 * Na endpointe typu klient staci posielat data pomocou send pretoze
 * identifikacia klienta sa nastavuje pri vytvarani socketu.
 */
class ZMQUtil {
public:
	/*
	 * Receive ZMQ string from socket and convert into string.
	 * The method is non-blocking.
	 */
	static int receive(Poco::SharedPtr<zmq::socket_t> socket,
		std::string &message);

	/*
	 * Convert string to ZMQ string and send to socket.
	 */
	static bool send(Poco::SharedPtr<zmq::socket_t> socket,
		const std::string &message);

	/*
	 *  Sends string as ZMQ string as multipart.
	 */
	static bool sendMultipart(Poco::SharedPtr<zmq::socket_t> socket,
		const std::string &message);
};

}

#endif
