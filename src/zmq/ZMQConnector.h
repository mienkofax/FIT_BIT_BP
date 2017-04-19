#ifndef BEEEON_ZMQ_CONNECTOR_H
#define BEEEON_ZMQ_CONNECTOR_H

#include <string>

#include <Poco/AtomicCounter.h>
#include <Poco/SharedPtr.h>

#include <zmq.hpp>

#include "loop/StoppableRunnable.h"
#include "util/Loggable.h"
#include "zmq/ZMQMessageError.h"

namespace BeeeOn {

class ZMQMessage;

/*
 * Abstract class for zmq server and client.
 * Obsahuje spolocne metody pre zmq clienta a zmq server. Pre jej
 * plnu funkcnost treba doplnit run metodu, kde sa vykonava beh
 * programu. Pre prijem dat su pripravene rozhrania dataServerReceive
 * a helloServerReceive. Pre konfiguraciu socketov je pripravne rozhranie
 * configureDataSockets a configureHelloSockets.
 *
 * Connector obsahuje dva sockety. Hello socket sluzi na priradenie ID, aby
 * bolo mozne komunikovat pomocou datoveho socketu asynchronne. Datovy
 * socket sluzi pre posielanie dat oboma smermi asynchronne.
 *
 * Defaultne je nastavene jedno vlakno pre spracovanie poziadaviek a tato
 * moznost sa neda zmenit.
 *
 * Vytvoreny zmq endpointu touto triedou umoznuje komunikovat
 * len pomocou tcp transportu, ktory je nastveny v metode createAddress().
 */
class ZMQConnector : public StoppableRunnable, public Loggable {
public:
	ZMQConnector();
	~ZMQConnector();

	void stop() override;

	void setDataServerHost(const std::string &host);
	void setHelloServerHost(const std::string &host);
	void setDataServerPort(const int port);
	void setHelloServerPort(const int port);

protected:
	/*
	 * Processing of messages from Device Manager (measured values,
	 * command)
	 */
	virtual void dataServerReceive() = 0;

	/*
	 * Processing of hello request message from Device Manager and
	 * create identification  for communication over dataSocket.
	 */
	virtual void helloServerReceive() = 0;

	/*
	 * It creates an endpoint address for accepting connections.
	 */
	std::string createAddress(const std::string &host, int port);

	virtual void configureDataSockets() = 0;
	virtual void configureHelloSockets() = 0;

	/*
	 * Dostane spravu a pokusi sa ju rozparsovat a zistit ci je mozne
	 * spravu poslat dalej. Metoda zarucuje len obsah atributu messageType.
	 * Chyby sposobene neexistujucim inym atributom je potrebne overit
	 * samostatne.
	 */
	bool parseMessage(const std::string &jsonMessage,
		Poco::SharedPtr<zmq::socket_t> socket, ZMQMessage &msg);

	int sendError(const ZMQMessageError::Error errorType,
		const std::string message, Poco::SharedPtr<zmq::socket_t> socket);

protected:
	Poco::AtomicCounter m_stop;
	std::string m_dataServerHost;
	std::string m_helloServerHost;
	int m_dataServerPort;
	int m_helloServerPort;
	zmq::context_t m_context;

	Poco::SharedPtr<zmq::socket_t> m_dataServerSocket;
	Poco::SharedPtr<zmq::socket_t> m_helloServerSocket;
};

}

#endif
