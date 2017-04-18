#include <cppunit/extensions/HelperMacros.h>

#include "zmq/ZMQDeviceManagerTable.h"

using namespace std;

namespace BeeeOn {

class ZMQDeviceManagerTableTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ZMQDeviceManagerTableTest);
	CPPUNIT_TEST(testRegisterDaemonID);
	CPPUNIT_TEST_SUITE_END();

public:
	void testRegisterDaemonID();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ZMQDeviceManagerTableTest);

/*
 * It tests insertion of a new device manager into table and
 * correctness of a generated id. It also tests device unregistration.
 */
void ZMQDeviceManagerTableTest::testRegisterDaemonID()
{
	ZMQDeviceManagerTable deviceManagerTable;

	DeviceManagerID deviceManager1 =
		deviceManagerTable.registerDaemonPrefix(
			DevicePrefix::fromRaw(DevicePrefix::PREFIX_ZWAVE));

	DeviceManagerID deviceManager2 =
		deviceManagerTable.registerDaemonPrefix(
			DevicePrefix::fromRaw(DevicePrefix::PREFIX_ZWAVE));

	CPPUNIT_ASSERT(deviceManager1.prefix() == DevicePrefix::PREFIX_ZWAVE);
	CPPUNIT_ASSERT(deviceManager2.prefix() == DevicePrefix::PREFIX_ZWAVE);
	CPPUNIT_ASSERT(deviceManager1.ident() == 0);
	CPPUNIT_ASSERT(deviceManager2.ident() == 1);

	CPPUNIT_ASSERT(deviceManagerTable.count() == 2);
	CPPUNIT_ASSERT(deviceManager1 != deviceManager2);

	deviceManagerTable.unregisterDaemon(deviceManager2);
	CPPUNIT_ASSERT(deviceManagerTable.count() == 1);
}

}
