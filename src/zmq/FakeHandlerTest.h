#ifndef BEEEEON_FAKEHANDLER_TEST_H
#define BEEEEON_FAKEHANDLER_TEST_H

#include <Poco/Nullable.h>
#include <Poco/Timer.h>

#include "core/CommandDispatcher.h"
#include "core/CommandHandler.h"
#include "model/DeviceID.h"
#include "util/Loggable.h"

namespace BeeeOn {

class FakeHandlerTest : public CommandHandler, public Loggable {
public:
	FakeHandlerTest();

	bool accept(const Command::Ptr cmd);
	void handle(Command::Ptr cmd, Answer::Ptr answer);

	void setJablotronAC881(const std::string &paired);
	void setJablotronAC882(const std::string & paired);
	void setJablotronJA81M(const std::string & paired);
	void setJablotronJA82SH(const std::string & paired);
	void setJablotronJA83M(const std::string & paired);
	void setJablotronJA83P(const std::string & paired);
	void setJablotronJA85ST(const std::string & paired);
	void setJablotronRC86K(const std::string & paired);
	void setJablotronTP82N(const std::string & paired);

	void setJablotronAc881LastValue(int lastValue);
	void setJablotronAc882LastValue(int lastValue);

	void setZWaveAeotec(const std::string & paired);
	void setZWaveDLink(const std::string & paired);
	void setZWaveFibaro(const std::string & paired);
	void setZWavePhilio(const std::string & paired);
	void setZWavePopp(const std::string & paired);

	void setZWavePoppLastState(int lastValue);
	void setZWaveDLinkLastState(int lastValue);
	void setZWaveAeotecPirSensorLastValue(int lastValue);
	void setZWaveAeotecRefreshTimeLastValue(int lastValue);

	void setAction(const std::string& action);
	void setParameter1(const std::string &parameter);
	void setParameter2(const std::string &parameter);
	void setParameter3(const std::string &parameter);
	void setParameter4(const std::string &parameter);
	void setPrefixName(const std::string &prefixName);
	void setRunTime(int time);

	void setCommandDispatcher(Poco::SharedPtr<CommandDispatcher> dispatcher);
	void addPairedDeviceID(const DeviceID &deviceID);

private:
	Poco::Nullable<DeviceID> generateDeviceID(const DevicePrefix &prefix,
		const std::string &device);

	std::vector<DeviceID> pairedDevices(const DevicePrefix &prefix);

	void startAction(Poco::Timer &timer);
	void fire(Poco::Timer &timer);

private:
	Poco::Nullable<DeviceID> m_deviceAC881;
	Poco::Nullable<DeviceID> m_deviceAC882;
	int m_deviceAC881LastState;
	int m_deviceAC882LastState;

	Poco::Nullable<DeviceID> m_deviceAeotec;
	Poco::Nullable<DeviceID> m_deviceDLink;
	Poco::Nullable<DeviceID> m_deviceFibaro;
	Poco::Nullable<DeviceID> m_devicePhilio;
	Poco::Nullable<DeviceID> m_devicePopp;

	int m_devicePoppLastState;
	int m_deviceDLinkLastState;
	int m_deviceAeotecPirSensorLastValue;
	int m_deviceAeotecRefreshTimeLastValue;

	std::string m_action;
	std::string m_parameter1;
	std::string m_parameter2;
	std::string m_parameter3;
	std::string m_parameter4;
	std::string m_prefix;

	Poco::Timer m_defer;
	Poco::TimerCallback<FakeHandlerTest> m_callback;
	Poco::SharedPtr<CommandDispatcher> m_dispatcher;
	std::set<DeviceID> m_pairedDevice;
	Poco::AtomicCounter m_activeAction;
};

}


#endif
