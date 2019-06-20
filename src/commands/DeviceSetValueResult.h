#ifndef BEEEON_DEVICE_SET_VALUE_RESULT_H
#define BEEEON_DEVICE_SET_VALUE_RESULT_H

#include <Poco/Mutex.h>

#include "core/Answer.h"
#include "core/Result.h"

namespace BeeeOn {

class DeviceSetValueResult : public Result {
public:
	typedef Poco::AutoPtr<DeviceSetValueResult> Ptr;

	enum SetStatus {
		GW_DEVICE_SUCCESS,
		GW_DEVICE_FAILED,
		GW_DEVICE_TIMEOUT,
		DEVICE_SUCCESS,
		DEVICE_FAILED,
		DEVICE_TIMEOUT,
	};

	DeviceSetValueResult(const Answer::Ptr answer);

	void setExtendetSetStatus(SetStatus status);
	void setExtendetSetStatusUnlocked(SetStatus status);

	SetStatus extendetSetStatus() const;
	SetStatus extendetSetStatusUnlocked() const;

private:
	SetStatus m_status;
};

}

#endif
