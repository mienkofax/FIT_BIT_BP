#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <Poco/Delegate.h>
#include <Poco/NumberParser.h>
#include <Poco/RegularExpression.h>
#include <Poco/String.h>

#include "core/AnswerQueue.h"
#include "di/Injectable.h"
#include "jablotron/JablotronDeviceManager.h"
#include "util/ZMQUtil.h"
#include "zmq/ZMQMessage.h"

BEEEON_OBJECT_BEGIN(BeeeOn, JablotronDeviceManager)
BEEEON_OBJECT_CASTABLE(StoppableRunnable)
BEEEON_OBJECT_TEXT("dataServerHost", &JablotronDeviceManager::setDataServerHost)
BEEEON_OBJECT_NUMBER("dataServerPort", &JablotronDeviceManager::setDataServerPort)
BEEEON_OBJECT_TEXT("helloServerHost", &JablotronDeviceManager::setHelloServerHost)
BEEEON_OBJECT_NUMBER("helloServerPort", &JablotronDeviceManager::setHelloServerPort)
BEEEON_OBJECT_TEXT("prefixName", &JablotronDeviceManager::setPrefixName)
BEEEON_OBJECT_TEXT("donglePath", &JablotronDeviceManager::setDonglePath)
BEEEON_OBJECT_END(BeeeOn, JablotronDeviceManager)

using namespace BeeeOn;
using namespace std;
using namespace Poco;

#define DELAY_AFTER_SET_SWITCH      2
#define DELAY_BEETWEEN_CYCLES       400000
#define DELAY_BETWEEN_PARSE         10
#define FULL_BATTERY                100
#define JABLOTRON_MAINS_OUTLET      0
#define LOW_BATTERY                 5
#define MAX_DEVICES_IN_JABLOTRON    32
#define MAX_NUMBER_FAILED_REPEATS   10
#define NUMBER_OF_RETRIES           3
#define QUEUE_WAIT                  50000
#define SET_STATE                   2
#define UNKNOWN_STATE               0
#define UNSET_STATE                 1

JablotronDeviceManager::JablotronDeviceManager():
	DeviceManager(),
	m_sensorEvent(false),
	m_queueLoop(false),
	m_callback(*this, &JablotronDeviceManager::stopListen),
	m_listen(false),
	m_deferAfter(1000, 0)
{
	m_zmqClient->onReceive += delegate(this, &JablotronDeviceManager::onEvent);
}

void JablotronDeviceManager::run()
{
	bool loadDevices = false;
	string buffer;

	initJablotronSerial();
	runClient();

	sleep(1);
	getDeviceList();

	while (!m_stop) {
		checkQueue();

		if (m_devicesWithFlag.size() == 0)
			continue;

		if (!loadJablotronDevices(loadDevices)) {
			sleep(DELAY_BETWEEN_PARSE);
			continue;
		}

		if (!readFromSerial(buffer))
			continue;

		StringTokenizer token(trim(buffer), " ");
		if (token.count() < 3)
			continue;

		sendMeasuredValues(trim(buffer), token);
	}
}

void JablotronDeviceManager::getDeviceList()
{
	m_devicesWithFlag.clear();

	Answer::Ptr answer = new Answer(m_queue);
	ServerDeviceListCommand::Ptr cmd = new ServerDeviceListCommand(m_prefix);

	ZMQMessage msg = ZMQMessage::fromCommand(cmd);
	msg.setID(GlobalID::random());

	m_zmqClient->send(msg.toString());
	m_table.insert(make_pair(answer, ResultData{msg.id(), cmd}));

	logger().debug("run cmd: " + cmd->name());
}

void JablotronDeviceManager::getLastValue(const DeviceID &deviceID)
{
	Answer::Ptr answer = new Answer(m_queue);
	ServerLastValueCommand::Ptr cmd =
		new ServerLastValueCommand(deviceID, JABLOTRON_MAINS_OUTLET);

	ZMQMessage msg = ZMQMessage::fromCommand(cmd);
	msg.setID(GlobalID::random());

	m_zmqClient->send(msg.toString());
	m_table.insert(make_pair(answer, ResultData{msg.id(), cmd}));

	logger().debug("run cmd: " + cmd->name());
}

void JablotronDeviceManager::checkQueue()
{
	std::list<Answer::Ptr> dirtyList;
	m_queue.wait(QUEUE_WAIT, dirtyList);

	for (auto answer : dirtyList) {
		auto it = m_table.find(answer);

		if (it == m_table.end()) {
			logger().warning("unknown result");
			break;
		}

		if (it->second.cmd->is<ServerDeviceListCommand>()) {
			m_devicesWithFlag.clear();

			for (auto deviceID : answer->at(0).cast<ServerDeviceListResult>()->deviceList())
				m_devicesWithFlag.insert(deviceID);
		}

		if (it->second.cmd->is<ServerLastValueCommand>()) {
			if (answer->at(0).cast<ServerLastValueResult>()->status() != Result::SUCCESS) {
				return;
			}

			setSwitch(
				it->second.cmd.cast<ServerLastValueCommand>()->deviceID(),
				(short) answer->at(0).cast<ServerLastValueResult>()->value());
		}
	}
}

void JablotronDeviceManager::onEvent(const void *, ZMQMessage &zmqMessage)
{
	switch(zmqMessage.type().raw()){
	case ZMQMessageType::TYPE_DEVICE_LAST_VALUE_RESULT:
		doDeviceLastValueResult(zmqMessage);
		break;
	case ZMQMessageType::TYPE_DEVICE_LIST_RESULT:
		doTypeDeviceListResult(zmqMessage);
		break;
	case ZMQMessageType::TYPE_LISTEN_CMD:
		doListenCommand(zmqMessage);
		break;
	case ZMQMessageType::TYPE_DEVICE_UNPAIR_CMD:
		doDeviceUnpairCommand(zmqMessage);
		break;
	case ZMQMessageType::TYPE_SET_VALUES_CMD:
		doSetValuesCommand(zmqMessage);
		break;
	default:
		logger().error("unsupported cmd " + zmqMessage.type().toString());
	}
}

void JablotronDeviceManager::doSetValuesCommand(ZMQMessage &zmqMessage)
{
	DeviceSetValueCommand::Ptr cmd = zmqMessage.toDeviceSetValueCommand();

	setSwitch(
		cmd.cast<DeviceSetValueCommand>()->deviceID(),
		(short) cmd.cast<DeviceSetValueCommand>()->value());
}

void JablotronDeviceManager::doDeviceUnpairCommand(ZMQMessage &zmqMessage)
{
	DeviceUnpairCommand::Ptr cmd = zmqMessage.toDeviceUnpairCommand();
	m_devicesWithFlag.erase(cmd->deviceID());
}

void JablotronDeviceManager::doListenCommand(ZMQMessage &zmqMessage)
{
	GatewayListenCommand::Ptr cmd = zmqMessage.toGatewayListenCommand();
	m_deferAfter.setStartInterval(cmd->duration().totalMilliseconds());

	startListen(m_deferAfter);
}

void JablotronDeviceManager::doTypeDeviceListResult(ZMQMessage &zmqMessage)
{
	for (auto item : m_table) {
		if (item.second.id != zmqMessage.id())
			continue;

		ServerDeviceListResult::Ptr res = new ServerDeviceListResult(item.first);
		zmqMessage.toServerDeviceListResult(res);
	}
}

void JablotronDeviceManager::doDeviceLastValueResult(ZMQMessage &zmqMessage)
{
	for (auto item : m_table) {
		if (item.second.id != zmqMessage.id())
			continue;

		ServerLastValueResult::Ptr res = new ServerLastValueResult(item.first);
		zmqMessage.toServerLastValueResult(res);
	}
}

void JablotronDeviceManager::initJablotronSerial()
{
	try {
		m_serial.assign(new SerialControl(m_donglePath));
		logger().debug("initialization succesful");
	}
	catch (std::exception& ex) {
		logger().fatal("serial reset: " + (string)ex.what(), __FILE__, __LINE__);
		stop();
	}
}

bool JablotronDeviceManager::loadJablotronDevices(bool &loadDevices)
{
	if (!m_serial->isValidFd()) {
		if(!m_serial->initSerial()) {
			logger().error("Failed to open Jablotron device errno: " + to_string(errno)
					  + " "+ __FILE__ + to_string(__LINE__));
			loadDevices = false;
			return false;
		}
	}

	if (!loadDevices) {
		if (loadRegDevices())
			loadDevices = true;
		else
			return false;
	}

	return true;
}

JablotronDeviceManager::JablotronSerialNumber JablotronDeviceManager::getSerialNumber(
	const string &token)
{
	if (token.size() < 10)
		throw InvalidArgumentException("substr could not be performed");

	try {
		return NumberParser::parseUnsigned(token.substr(1, 8));
	}
	catch (const Exception& ex) {
		logger().log(ex, __FILE__, __LINE__);
		throw;
	}
}

int JablotronDeviceManager::getDeviceType(JablotronSerialNumber serialNumber) const
{
	if ((serialNumber >= 0xCF0000) && (serialNumber <= 0xCFFFFF))
		return AC88;

	if ((serialNumber >= 0x580000) && (serialNumber <= 0x59FFFF))
		return JA80L;

	if ((serialNumber >= 0x240000) && (serialNumber <= 0x25FFFF))
		return TP82N;

	if ((serialNumber >= 0x1C0000) && (serialNumber <= 0x1DFFFF))
		return JA83M;

	if ((serialNumber >= 0x180000) && (serialNumber <= 0x1BFFFF))
		return JA81M;

	if ((serialNumber >= 0x7F0000) && (serialNumber <= 0x7FFFFF))
		return JA82SH;

	if ((serialNumber >= 0x640000) && (serialNumber <= 0x65FFFF))
		return JA83P;

	if ((serialNumber >= 0x760000) && (serialNumber <= 0x76FFFF))
		return JA85ST;

	if ((serialNumber >= 0x800000) && (serialNumber <= 0x97FFFF))
		return RC86K;

	return 0;
}

bool JablotronDeviceManager::loadRegDevices()
{
	RegularExpression re("\\[([0-9]+)\\]");
	string msg;
	JablotronSerialNumber deviceSN;

	for (int i = 0; i < MAX_DEVICES_IN_JABLOTRON; i++) {
		vector<string> tmp;
		stringstream stream;

		// we need 2-digits long value (zero-prefixed when needed) - MAX_DEVICES_IN_JABLOTRON
		stream << setfill('0') << setw(2) << i;
		m_serial->ssend("\x1BGET SLOT:" + stream.str() + "\n");

		msg = m_serial->sread();
		if (re.split(msg, tmp) == 2) {
			try {
				deviceSN = NumberParser::parseUnsigned(tmp[1]);
			}
			catch(const Exception& ex) {
				logger().error("No conversion could be performed: " + ex.displayText());
				logger().log(ex, __FILE__, __LINE__);
				return false;
			}

			m_devices.push_back(deviceSN);
			logger().debug("device: " + to_string(deviceSN));
		}
	}

	for (auto deviceID : m_devicesWithFlag) {
		if (getDeviceType(deviceID.ident() & 0xffffff) == AC88)
			getLastValue(deviceID);
	}

	usleep(DELAY_BEETWEEN_CYCLES);
	return true;
}

bool JablotronDeviceManager::readFromSerial(std::string &data)
{
	RegularExpression re("(?!\\n)[^\\n]*(?=\\n)");
	vector<string> splitReceiveData;
	m_buffer += m_serial->sread();

	if (re.split(m_buffer, splitReceiveData) == 0)
		return false;

	m_buffer.erase(0, splitReceiveData.front().size() + 2);
	data = splitReceiveData.front();
	logger().debug("receive data: " + data);

	// OK or ERROR sending dongle whether the sent message is valid or not.
	// Ok Send when the dongle has nothing to send too.
	// Documentation: https://www.turris.cz/gadgets/manual#obecne_principy_komunikace
	return !(data == "OK" || data == "ERROR");
}

/**
 * Your event has been set but it is necessary to off what is achieved by adding
 * value to sensorEvent.values. First, send a message with Eventim agg->sendData(msg),
 * you need to change to not have the same timestamp (+2).
 */
void JablotronDeviceManager::sendMeasuredValues(const string &message,
	StringTokenizer &token)
{
	SensorData sensorData;

	try {
		sensorData = parseMessageFromDevice(message, token);
	}
	catch (const Exception& ex) {
		logger().log(ex, __FILE__, __LINE__);
		return;
	}

	auto it = m_devicesWithFlag.find(sensorData.deviceID());
	if (it == m_devicesWithFlag.end() && !m_listen) {
		logger().debug("drop message");
		return;
	}

	m_zmqClient->send(ZMQMessage::fromSensorData(sensorData).toString());

	if (m_sensorEvent) {
		sleep(1);
		SensorData sensorDataEvent;
		sensorDataEvent.setDeviceID(sensorData.deviceID());
		sensorData.insertValue(m_sensorEventValue);

		m_zmqClient->send(ZMQMessage::fromSensorData(sensorDataEvent).toString());
		m_sensorEvent = false;
	}
}

SensorData JablotronDeviceManager::parseMessageFromDevice(
	const string &message, StringTokenizer &token)
{
	ModuleID batteryModuleID(RC86K_ModuleID::BATTERY_STATE);
	JablotronSerialNumber serialNumber = getSerialNumber(token[0]);
	int deviceType = getDeviceType(serialNumber);

	SensorData sensorData;
	sensorData.setDeviceID(createDeviceID(serialNumber));

	if (deviceType == AC88) {
		sensorData.insertValue(SensorValue(
			ModuleID(AC88_ModuleID::SENSOR_STATE),
			convert(token[2], false)));
		return sensorData;
	}

	if (deviceType == JA85ST)
		batteryModuleID = ModuleID(JA85ST_ModuleID::BATTERY_STATE);

	sensorData.insertValue(SensorValue(
		batteryModuleID,
		getBatteryStatus(token[3])));

	switch (deviceType) {
	case JA81M:
	case JA83M:
		sensorData.insertValue(parseMessageFromJA81M_JA83M(token));
		break;
	case JA83P:
	case JA82SH:
		sensorData.insertValue(parseMessageFromJA83P_JA82SH(token));
		break;
	case JA85ST:
		sensorData.insertValue(parseMessageFromJA85ST(token));
		break;
	case RC86K:
		sensorData.insertValue(parseMessageFromRC86K(token[2]));
		break;
	case TP82N:
		sensorData.insertValue(parseMessageFromTP82N(message, token[2]));
		break;
	default:
		throw InvalidArgumentException("unknown jablotron device");
	}

	return sensorData;
}

SensorValue JablotronDeviceManager::parseMessageFromJA85ST(
	const StringTokenizer& token)
{
	if (token[2] == "SENSOR") {
		m_sensorEvent = true;

		m_sensorEventValue = SensorValue(
			ModuleID(JA85ST_ModuleID::FIRE_SENSOR),
			FIRE_SENSOR_VALUE::STATE_OFF);

		return SensorValue(
			ModuleID(JA85ST_ModuleID::FIRE_SENSOR),
			FIRE_SENSOR_VALUE::STATE_ON);
	}
	else if (token[2] == "BUTTON") {
		return SensorValue(
			ModuleID(JA85ST_ModuleID::FIRE_SENSOR),
			SENSOR_VALUE::STATE_ON);
	}
	else if (token[2] == "TAMPER") {
		return SensorValue(
			ModuleID(JA85ST_ModuleID::FIRE_SENSOR_ALARM),
			convert(token[4]));
	}
	else if (token[2] == "DEFECT") {
		return SensorValue(
			ModuleID(JA85ST_ModuleID::FIRE_SENSOR_ERROR),
			convert(token[4]));
	}

	throw InvalidArgumentException("unknown message attribute");
}

SensorValue JablotronDeviceManager::parseMessageFromJA83P_JA82SH(
	const StringTokenizer& token)
{
	if (token[2] == "TAMPER") {
		return SensorValue(
			ModuleID(JA83P_JA82SH_ModuleID::SENSOR_ALARM),
			convert(token[4]));
	}
	else if (token[2] == "SENSOR") {
		m_sensorEvent = true;

		m_sensorEventValue = SensorValue(
			ModuleID(JA83P_JA82SH_ModuleID::SENSOR),
			SENSOR_VALUE::STATE_OFF);

		return SensorValue(
			ModuleID(JA83P_JA82SH_ModuleID::SENSOR),
			SENSOR_VALUE::STATE_ON);
	}

	throw InvalidArgumentException("unknown message attribute");
}

SensorValue JablotronDeviceManager::parseMessageFromJA81M_JA83M(
	const StringTokenizer& token)
{
	if (token[2] == "SENSOR") {
		return SensorValue(
			ModuleID(JA81M_JA83M_ModuleID::DOOR_CONTACT),
			convert(token[4]));
	}
	else if (token[2] == "TAMPER") {
		return SensorValue(
			ModuleID(JA81M_JA83M_ModuleID::DOOR_CONTACT_ALARM),
			convert(token[4]));
	}

	throw InvalidArgumentException("unknown message attribute");
}

SensorValue JablotronDeviceManager::parseMessageFromRC86K(
	const string& data)
{
	if (data == "PANIC") {
		m_sensorEvent = true;

		m_sensorEventValue = SensorValue(
			ModuleID(RC86K_ModuleID::REMOTE_CONTROL_ALARM),
			SENSOR_VALUE::STATE_OFF);

		return SensorValue(
			ModuleID(RC86K_ModuleID::REMOTE_CONTROL_ALARM),
			SENSOR_VALUE::STATE_ON);
	}
	else {
		return SensorValue(
			ModuleID(RC86K_ModuleID::REMOTE_CONTROL),
			convert(data));
	}
}

SensorValue JablotronDeviceManager::parseMessageFromTP82N(
	const string &message, const string &data)
{
	StringTokenizer tokenIntSet(data, ":");
	string temperature;

	if (message.size() < 26)
		throw InvalidArgumentException("substr could not be performed");

	temperature = message.substr(22, 4);

	if (tokenIntSet[0] == "SET")
		return SensorValue(
			ModuleID(TP82N_ModuleID::REQUESTD_ROOM_TEMPERATURE),
			NumberParser::parseFloat(temperature));
	else
		return SensorValue(
			ModuleID(TP82N_ModuleID::CURRENT_ROOM_TEMPERATURE),
			NumberParser::parseFloat(temperature));
}

/* Retransmission packet status is recommended to be done 3 times with
 * a minimum gap 200ms and 500ms maximum space (for an answer) - times
 * in the space, it is recommended to choose random.
 */
void JablotronDeviceManager::retransmissionPacket(const string& msg)
{
	int maximum = 0;
	for (short i = 0; i < NUMBER_OF_RETRIES && maximum < MAX_NUMBER_FAILED_REPEATS; i++) {
		if (!m_serial->ssend(msg)) {
			maximum++;
			i--;
			continue;
		}

		logger().debug("changed: " + msg);
		usleep(DELAY_BEETWEEN_CYCLES);
	}

	sleep(DELAY_AFTER_SET_SWITCH);
}

void JablotronDeviceManager::setSwitch(const DeviceID &deviceID, short sw)
{
	JablotronSerialNumber ac88_sn_pgx = 0;

	for (auto &device : m_devices) {
		if ((getDeviceType(device) == AC88) && (ac88_sn_pgx == 0))
			ac88_sn_pgx = device;
	}

	if (createDeviceID(ac88_sn_pgx) == deviceID)
		pgx = sw;
	else
		pgy = sw;

	string setString = "\x1BTX ENROLL:0 PGX:" + to_string(pgx) +
		" PGY:" + to_string(pgy) + " ALARM:0 BEEP:FAST\n";

	logger().debug("set switch: " + setString);
	retransmissionPacket(setString);
}

double JablotronDeviceManager::getValue(const string& msg) const
{
	StringTokenizer token(msg, ":");
	return NumberParser::parseFloat(token[1]);
}

double JablotronDeviceManager::convert(const string& data, bool reverse) const
{
	if (getValue(data) == 1)
		return UNSET_STATE;

	if (reverse)
		return SET_STATE;

	return UNKNOWN_STATE;
}

int JablotronDeviceManager::getBatteryStatus(const string& msg) const
{
	return getValue(msg) ? LOW_BATTERY : FULL_BATTERY;
}

DeviceID JablotronDeviceManager::createDeviceID(JablotronSerialNumber sn) const
{
	return DeviceID(DevicePrefix::fromRaw(DevicePrefix::PREFIX_JABLOTRON), sn);
}

void JablotronDeviceManager::setDonglePath(const std::string &path)
{
	m_donglePath = path;
}

void JablotronDeviceManager::startListen(Timer &timer)
{
	logger().debug("start listen");
	m_listen = true;
	timer.start(m_callback);
}

void JablotronDeviceManager::stopListen(Timer &)
{
	logger().debug("stop listen");
	m_listen = false;
	getDeviceList();
}
