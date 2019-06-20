#include "commands/DeviceSetValueResult.h"

using namespace BeeeOn;

DeviceSetValueResult::DeviceSetValueResult(const Answer::Ptr answer):
	Result(answer)
{
}

void DeviceSetValueResult::setExtendetSetStatus(
	DeviceSetValueResult::SetStatus status)
{
	Poco::FastMutex::ScopedLock guard(lock());
	setExtendetSetStatusUnlocked(status);
}

void DeviceSetValueResult::setExtendetSetStatusUnlocked(
	DeviceSetValueResult::SetStatus status)
{
	assureLocked();
	m_status = status;
}

DeviceSetValueResult::SetStatus DeviceSetValueResult::extendetSetStatus() const
{
	Poco::FastMutex::ScopedLock guard(lock());
	return extendetSetStatusUnlocked();
}

DeviceSetValueResult::SetStatus DeviceSetValueResult::extendetSetStatusUnlocked() const
{
	assureLocked();
	return m_status;
}
