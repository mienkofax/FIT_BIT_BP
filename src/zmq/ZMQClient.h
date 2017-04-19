#ifndef BEEEON_ZMQ_CLIENT_H
#define BEEEON_ZMQ_CLIENT_H

#include <Poco/BasicEvent.h>
#include <Poco/Nullable.h>

#include "model/DeviceManagerID.h"
#include "model/DevicePrefix.h"
#include "zmq/ZMQConnector.h"

namespace BeeeOn {

/*
 * Trieda umoznujuca pripojenie k zmq serveru a komunikaciu s nim.
 * Umoznuje prijmat a posielat spravy, ktore parsuje. Tato trieda
 * by nemala obsahovat spracovanie prikazov, ktore su specificke pre
 * dany device managerov.ZMQMessage Na spracovanie tychto sprav je potrebne
 * sa prihlasit na odoberanie eventu onReceive.
 *
 * Trieda obsahuje dva sockety. hello socket sluzi pre ziskanie
 * device manage ID. Datovy socket sluzi na zasielanie dat,
 * ktoré sú potrebne pre branu alebo server.
 *
 * Client si po starte vyziada od servera device manager ID, ktorym
 * bude dalej identifikovany na datovom sockete (m_dataServerSocket).
 */
class ZMQClient : public ZMQConnector {
public:
	ZMQClient();

	void run() override;

public:
	Poco::BasicEvent<ZMQMessage> onReceive;

	void setDeviceManagerPrefix(const DevicePrefix &prefix);

	Poco::Nullable<DeviceManagerID> deviceManagerID();

	int send(const std::string &message);

private:
	void configureDataSockets() override;
	void configureHelloSockets() override;

	void dataServerReceive() override;
	void helloServerReceive() override;

private:
	Poco::Nullable<DeviceManagerID> m_deviceMangerID;
	DevicePrefix m_devicePrefix;

};

}

#endif
