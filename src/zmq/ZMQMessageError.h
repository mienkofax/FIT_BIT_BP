#ifndef BEEEON_ZMQ_PROTOCOL_MESSAGE_ERROR_H
#define BEEEON_ZMQ_PROTOCOL_MESSAGE_ERROR_H

#include <string>

namespace BeeeOn {

/*
 * Error representation that is sent using zmq. The error
 * is identified by an error code and an error message.
 */
class ZMQMessageError {
public:
	enum Error {
		ERROR_JSON_SYNTAX,
		ERROR_MISSING_ATTRIBUTE,
		ERROR_UNSUPPORTED_MESSAGE,
	};

	ZMQMessageError(const Error errorCode,
		const std::string &errorMessage);

	Error errorCode() const;
	std::string errorMessage() const;

private:
	Error m_errorCode;
	std::string m_errorMessage;
};

}

#endif
