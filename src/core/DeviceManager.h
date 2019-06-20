#ifndef BEEEON_DEVICE_MANAGER_H
#define BEEEON_DEVICE_MANAGER_H

#include <Poco/AtomicCounter.h>
#include <Poco/SharedPtr.h>

#include "core/Answer.h"
#include "core/Command.h"
#include "loop/StoppableRunnable.h"
#include "loop/LoopRunner.h"
#include "model/DeviceID.h"
#include "model/GlobalID.h"
#include "util/Loggable.h"
#include "zmq/ZMQClient.h"

namespace BeeeOn {

class DeviceManager : public Loggable, public StoppableRunnable {
public:
	DeviceManager();

	void stop();

	void setDataServerHost(const std::string &host);
	void setHelloServerHost(const std::string &host);
	void setDataServerPort(const int port);
	void setHelloServerPort(const int port);
	void setPrefixName(const std::string &prefixName);

protected:
	/*
	 * Struktura reprezentujuca potrebne udaje ktore sa ulozia do mapy
	 * m_table spolu s answer a su potrebne pre prijatie spravne
	 * priradenie prijatej spravy.
	 */
	struct ResultData {
		GlobalID id;
		Command::Ptr cmd;
	};

	/*
	 * Rozhranie pre prijem dat.
	 */
	virtual void onEvent(const void*, ZMQMessage &zmqMessage) = 0;

	void runClient();

protected:
	Poco::AtomicCounter m_stop;
	DevicePrefix m_prefix;
	LoopRunner m_runner;
	std::map<Answer::Ptr, ResultData> m_table;
	Poco::SharedPtr<ZMQClient> m_zmqClient;
};

}


#endif
