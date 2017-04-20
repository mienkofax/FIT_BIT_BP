#ifndef BEEEON_ZMQ_PROTOCOL_MESSAGE_H
#define BEEEON_ZMQ_PROTOCOL_MESSAGE_H

#include <string>

#include <Poco/JSON/Object.h>

#include "model/DeviceManagerID.h"
#include "model/DevicePrefix.h"
#include "zmq/ZMQMessageError.h"
#include "zmq/ZMQMessageType.h"
#include "zmq/ZMQMessageValueType.h"

namespace BeeeOn {

class DeviceID;
class ModuleID;
class SensorData;
class SensorValue;

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
 *
 * Example for receiving message (client):
 *     ZMQMessage m = ZMQMessage::fromJSON(input);
 *
 *     if (m.type() == ZMQMessageType::TYPE_ERROR)
 *         ZMQMessageError error = m.toError();
 *
 * Example for sending message (server):
 *     ZMQMessage m = ZMQMessage::createError();
 *     zmqServer.send(m);
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

	ZMQMessageError toError();

	DevicePrefix toHelloRequest();

	DeviceManagerID toHelloResponse();

	SensorData toSensorData();

	/*
	 * Parses json message and store into Poco::JSON::Object (m_json).
	 */
	static ZMQMessage fromJSON(const std::string &json);

	static ZMQMessage fromSensorData(const SensorData &sensorData);

	/*
	 * It is a response to an unknown message/attribute.
	 */
	static ZMQMessage fromError(const ZMQMessageError::Error error,
		const std::string &message);

	static ZMQMessage fromHelloRequest(const DevicePrefix &devicePrefix);

	static ZMQMessage fromHelloResponse(const DeviceManagerID &deviceManagerID);

private:
	Poco::JSON::Object::Ptr jsonObject() const;

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

	/*
	 * {
	 *     "error_code" : 0
	 * }
	 */
	void setErrorCode(const ZMQMessageError::Error &error);
	ZMQMessageError::Error getErrorCode();

	/*
	 * {
	 *     "error_message" : "Error message in ..."
	 * }
	 */
	void setErrorMessage(const std::string errorMessage);
	std::string getErrorMessage();

	/*
	 * {
	 *     "device_manager_prefix" : "Fitprotocol"
	 * }
	 */
	void setDeviceManagerPrefix(const DevicePrefix &devicePrefix);
	DevicePrefix getDevicePrefix(Poco::JSON::Object::Ptr jsonObject);

	/*
	 * {
	 *     "device_manager_id" : "0xa00"
	 * }
	 */
	void setDeviceManagerID(const DeviceManagerID &deviceManagerID);

	/*
	 * {
	 *     "device_id" : "0x12345678945"
	 * }
	 */
	void setDeviceID(Poco::JSON::Object::Ptr jsonObject,
		const DeviceID &deviceID);
	DeviceID getDeviceID(Poco::JSON::Object::Ptr jsonObject);

	/*
	 * {
	 *     "module_id" : 0
	 * }
	 */
	void setModuleID(Poco::JSON::Object::Ptr jsonObject,
		const ModuleID &moduleID);
	ModuleID getModuleID(Poco::JSON::Object::Ptr jsonObject);

	/*
	 * {
	 *     "raw:" : "103.5"
	 * }
	 */
	void setRawValue(Poco::JSON::Object::Ptr jsonObject,
		const std::string &value);
	std::string getRawValue(Poco::JSON::Object::Ptr jsonObject);

	/*
	 * {
	 *     "type" : "double"
	 * }
	 */
	void setRawValueType(Poco::JSON::Object::Ptr jsonObject,
		const ZMQMessageValueType &valueType);
	ZMQMessageValueType getRawValueType(Poco::JSON::Object::Ptr jsonObject);

	/*
	 * True ak je raw hodnota daneho typu.
	 *
	 * {
	 *     "raw:" : "103.5",
	 *     "type" : "double"
	 * }
	 */
	void setValue(Poco::JSON::Object::Ptr jsonObject, const double value);
	bool getValue(Poco::JSON::Object::Ptr jsonObject, double &value);

	/*
	 * SensorValue
	 */
	void setSensorValue(Poco::JSON::Object::Ptr jsonObject,
		const SensorValue &sensorValue);
	SensorValue getSensorValue(Poco::JSON::Object::Ptr jsonObject);

private:
	Poco::JSON::Object::Ptr m_json;
};

}

#endif
