#include <Poco/Logger.h>
#include <Poco/NumberParser.h>

#include "commands/ServerDeviceListCommand.h"
#include "commands/ServerDeviceListResult.h"
#include "commands/ServerLastValueCommand.h"
#include "commands/ServerLastValueResult.h"
#include "commands/GatewayListenCommand.h"
#include "core/AnswerQueue.h"
#include "di/Injectable.h"
#include "zmq/FakeHandlerTest.h"
#include "zmq/ZMQMessage.h"

BEEEON_OBJECT_BEGIN(BeeeOn, FakeHandlerTest)
BEEEON_OBJECT_CASTABLE(CommandHandler)
BEEEON_OBJECT_REF("commandDispatcher", &FakeHandlerTest::setCommandDispatcher)
BEEEON_OBJECT_TEXT("setAction", &FakeHandlerTest::setAction)
BEEEON_OBJECT_TEXT("setParameter1", &FakeHandlerTest::setParameter1)
BEEEON_OBJECT_TEXT("setParameter2", &FakeHandlerTest::setParameter2)
BEEEON_OBJECT_TEXT("setParameter3", &FakeHandlerTest::setParameter3)
BEEEON_OBJECT_TEXT("setParameter4", &FakeHandlerTest::setParameter4)
BEEEON_OBJECT_NUMBER("setRunTime", &FakeHandlerTest::setRunTime)
BEEEON_OBJECT_TEXT("setPrefixName", &FakeHandlerTest::setPrefixName)
BEEEON_OBJECT_TEXT("setAC881", &FakeHandlerTest::setJablotronAC881)
BEEEON_OBJECT_NUMBER("setAC881LastValue", &FakeHandlerTest::setJablotronAc881LastValue)
BEEEON_OBJECT_TEXT("setAC882", &FakeHandlerTest::setJablotronAC882)
BEEEON_OBJECT_NUMBER("setAC882LastValue", &FakeHandlerTest::setJablotronAc882LastValue)
BEEEON_OBJECT_TEXT("setJA81M", &FakeHandlerTest::setJablotronJA81M)
BEEEON_OBJECT_TEXT("setJA82SH", &FakeHandlerTest::setJablotronJA82SH)
BEEEON_OBJECT_TEXT("setJA83M", &FakeHandlerTest::setJablotronJA83M)
BEEEON_OBJECT_TEXT("setJA83P", &FakeHandlerTest::setJablotronJA83P)
BEEEON_OBJECT_TEXT("setJA85ST", &FakeHandlerTest::setJablotronJA85ST)
BEEEON_OBJECT_TEXT("setRC86K", &FakeHandlerTest::setJablotronRC86K)
BEEEON_OBJECT_TEXT("setTP82N", &FakeHandlerTest::setJablotronTP82N)
BEEEON_OBJECT_TEXT("setAeotec", &FakeHandlerTest::setZWaveAeotec)
BEEEON_OBJECT_NUMBER("setAeotecPirSensorLastValue", &FakeHandlerTest::setZWaveAeotecPirSensorLastValue)
BEEEON_OBJECT_NUMBER("setAeotecRefreshTimeLastValue", &FakeHandlerTest::setZWaveAeotecRefreshTimeLastValue)
BEEEON_OBJECT_TEXT("setDLink", &FakeHandlerTest::setZWaveDLink)
BEEEON_OBJECT_NUMBER("setDLinkLastState", &FakeHandlerTest::setZWaveDLinkLastState)
BEEEON_OBJECT_TEXT("setFibaro", &FakeHandlerTest::setZWaveFibaro)
BEEEON_OBJECT_TEXT("setPhilio", &FakeHandlerTest::setZWavePhilio)
BEEEON_OBJECT_TEXT("setPopp", &FakeHandlerTest::setZWavePopp)
BEEEON_OBJECT_NUMBER("setPoppLastState", &FakeHandlerTest::setZWavePoppLastState)
BEEEON_OBJECT_END(BeeeOn, FakeHandlerTest)

using namespace BeeeOn;
using namespace Poco;
using namespace std;

const DevicePrefix jablotronPrefix = DevicePrefix::parse("Jablotron");
const DevicePrefix zwavePrefix = DevicePrefix::parse("Z-Wave");

FakeHandlerTest::FakeHandlerTest() :
	CommandHandler("FakeHandlerTest"),
	m_defer(8000, 0),
	m_callback(*this, &FakeHandlerTest::fire),
	m_activeAction(true)
{
}

bool FakeHandlerTest::accept(const Command::Ptr cmd)
{
	if (cmd->is<ServerDeviceListCommand>())
		return true;
	else if (cmd->is<ServerLastValueCommand>())
		return true;

	return false;
}

void FakeHandlerTest::handle(Command::Ptr cmd, Answer::Ptr answer)
{
	logger().debug("handle: "  + cmd->name());

	if (cmd->is<ServerDeviceListCommand>()) {
		logger().debug("handle: server device list ");
		if (m_activeAction) {
			startAction(m_defer);
			m_activeAction = false;
		}

		ServerDeviceListResult::Ptr result = new ServerDeviceListResult(answer);
		std::vector<DeviceID> deviceList =
			pairedDevices(cmd.cast<ServerDeviceListCommand>()->devicePrefix());

		Poco::FastMutex::ScopedLock guard(result->lock());
		result->setDeviceListUnlocked(deviceList);
		result->setStatusUnlocked(Result::SUCCESS);
	}
	else if (cmd->is<ServerLastValueCommand>()) {
		ServerLastValueResult::Ptr result = new ServerLastValueResult(answer);
		DeviceID deviceID = cmd->cast<ServerLastValueCommand>().deviceID();
		ModuleID moduleID = cmd->cast<ServerLastValueCommand>().moduleID();

		Poco::FastMutex::ScopedLock guard(result->lock());
		if (deviceID.prefix() == jablotronPrefix) {
			if (deviceID == m_deviceAC881) {
				result->setValueUnlocked(m_deviceAC881LastState);
				result->setStatusUnlocked(Result::SUCCESS);
			} else if (deviceID == m_deviceAC882) {
				result->setValueUnlocked(m_deviceAC882LastState);
				result->setStatusUnlocked(Result::SUCCESS);
			} else {
				result->setValueUnlocked(0);
				result->setStatusUnlocked(Result::FAILED);

				throw Poco::InvalidArgumentException(
					"unknown DeviceID: " + deviceID.toString());
			}

			result->notifyUpdated();
		}
		else {
			if (deviceID == m_devicePopp) {
				result->setValueUnlocked(m_devicePoppLastState);
				result->setStatusUnlocked(Result::SUCCESS);
			}
			else if (deviceID == m_deviceDLink) {
				result->setValueUnlocked(m_deviceDLinkLastState);
				result->setStatusUnlocked(Result::SUCCESS);
			}
			else if (deviceID == m_deviceAeotec && moduleID == ModuleID(6)) {
				result->setValueUnlocked(m_deviceAeotecPirSensorLastValue);
				result->setStatusUnlocked(Result::SUCCESS);
			}
			else if (deviceID == m_deviceAeotec && moduleID == ModuleID(8)) {
				result->setValueUnlocked(m_deviceAeotecRefreshTimeLastValue);
				result->setStatusUnlocked(Result::SUCCESS);
			}

			result->notifyUpdated();
		}
	}
	else {
		logger().warning("unknown handle command: " + cmd->name());
	}
}

Nullable<DeviceID> FakeHandlerTest::generateDeviceID(
	const DevicePrefix &prefix, const string &device)
{
	uint64_t number = NumberParser::parseUnsigned64(device);

	if (number == 0)
		return Nullable<DeviceID>();

	if (prefix == zwavePrefix)
		return DeviceID(prefix, (uint64_t(0xef1f4302) << 8) | unsigned(number));

	return DeviceID(prefix, number);
}

void FakeHandlerTest::startAction(Poco::Timer &timer)
{
	timer.start(m_callback);
}

void FakeHandlerTest::fire(Timer &)
{
	AnswerQueue queue;
	Answer::Ptr answer = new Answer(queue);
	Result::Ptr result = new Result(answer);
	Command::Ptr cmd;

	logger().debug("run action: " + m_action);

	if (m_action == "listen") {
		cmd = new GatewayListenCommand(
				NumberParser::parse(m_parameter1) * Timespan::SECONDS);
	}
	else if (m_action == "unpair") {
		DeviceID deviceID = DeviceID(
			DevicePrefix::parse(m_prefix), NumberParser::parseUnsigned64(m_parameter1));

		cmd = new DeviceUnpairCommand(deviceID);
	}
	else if (m_action == "set_value") {
		DeviceID deviceID;
		if (m_prefix == jablotronPrefix.toString()) {
			deviceID = DeviceID(
				DevicePrefix::parse(m_prefix), NumberParser::parseUnsigned64(m_parameter1));
		}
		else {
			deviceID = DeviceID(zwavePrefix, (uint64_t(0xef1f4302) << 8) | unsigned(NumberParser::parseUnsigned64(m_parameter1)));
		}

		 cmd = new DeviceSetValueCommand(
			 	deviceID,
			 ModuleID::parse(m_parameter2),
			 NumberParser::parseFloat(m_parameter3),
			 NumberParser::parse(m_parameter4)*Poco::Timespan::SECONDS);
	}
	else {
		return;
	}

	m_dispatcher->dispatch(cmd, answer);
	logger().debug("end action: " + m_action);
}

void FakeHandlerTest::setCommandDispatcher(
	SharedPtr<CommandDispatcher> dispatcher)
{
	m_dispatcher = dispatcher;
}

vector<DeviceID> FakeHandlerTest::pairedDevices(
	const DevicePrefix &prefix)
{
	vector<DeviceID> deviceList;

	for (auto deviceID : m_pairedDevice) {
		if (deviceID.prefix() == prefix)
			deviceList.push_back(deviceID);
	}

	return deviceList;
}

void FakeHandlerTest::addPairedDeviceID(const DeviceID &deviceID)
{
	m_pairedDevice.insert(deviceID);
}

// settings methods

void FakeHandlerTest::setJablotronAC881(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);

	m_deviceAC881 = deviceid;
}

void FakeHandlerTest::setJablotronAC882(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);

	m_deviceAC882 = deviceid;
}

void FakeHandlerTest::setJablotronJA81M(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setJablotronJA82SH(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setJablotronJA83M(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setJablotronJA83P(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setJablotronJA85ST(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setJablotronRC86K(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setJablotronTP82N(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(jablotronPrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setZWaveAeotec(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(zwavePrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);

	m_deviceAeotec = deviceid;
}

void FakeHandlerTest::setZWaveDLink(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(zwavePrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);

	m_deviceDLink = deviceid;
}

void FakeHandlerTest::setZWaveFibaro(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(zwavePrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setZWavePhilio(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(zwavePrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);
}

void FakeHandlerTest::setZWavePopp(const string &serialNumber)
{
	Nullable<DeviceID> deviceid = generateDeviceID(zwavePrefix, serialNumber);
	if (!deviceid.isNull())
		m_pairedDevice.insert(deviceid);

	m_devicePopp = deviceid;
}

void FakeHandlerTest::setJablotronAc881LastValue(int lastValue)
{
	m_deviceAC881LastState = lastValue;
}

void FakeHandlerTest::setJablotronAc882LastValue(int lastValue)
{
	m_deviceAC882LastState = lastValue;
}

void FakeHandlerTest::setPrefixName(const std::string &prefixName)
{
	m_prefix = prefixName;
}

void FakeHandlerTest::setParameter1(const std::string &parameter)
{
	m_parameter1 = parameter;
}

void FakeHandlerTest::setParameter2(const string &parameter)
{
	m_parameter2 = parameter;
}

void FakeHandlerTest::setParameter3(const string &parameter)
{
	m_parameter3 = parameter;
}

void FakeHandlerTest::setParameter4(const string &parameter)
{
	m_parameter4 = parameter;
}

void FakeHandlerTest::setAction(const string &action)
{
	if (action == "listen" || action == "unpair" || action == "set_value")
		m_action = action;
	else
		throw Poco::InvalidArgumentException(action);
}

void FakeHandlerTest::setRunTime(int time)
{
	m_defer.setStartInterval(time * Timespan::MILLISECONDS);
}

void FakeHandlerTest::setZWavePoppLastState(int lastValue)
{
	 m_devicePoppLastState = lastValue;
}

void FakeHandlerTest::setZWaveDLinkLastState(int lastValue)
{
	m_deviceDLinkLastState = lastValue;
}

void FakeHandlerTest::setZWaveAeotecPirSensorLastValue(int lastValue)
{
	m_deviceAeotecPirSensorLastValue = lastValue;
}

void FakeHandlerTest::setZWaveAeotecRefreshTimeLastValue(int lastValue)
{
	m_deviceAeotecRefreshTimeLastValue = lastValue;
}
