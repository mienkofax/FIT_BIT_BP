#include "zmq/ZMQMessageError.h"

using namespace BeeeOn;
using namespace std;

ZMQMessageError::ZMQMessageError(
		const ZMQMessageError::Error errorCode,
		const string &errorMessage):
	m_errorCode(errorCode),
	m_errorMessage(errorMessage)
{
}

ZMQMessageError::Error ZMQMessageError::errorCode() const
{
	return m_errorCode;
}

string ZMQMessageError::errorMessage() const
{
	return m_errorMessage;
}
