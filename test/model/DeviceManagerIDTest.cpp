#include <cppunit/extensions/HelperMacros.h>

#include "model/DeviceManagerID.h"

namespace BeeeOn {

class DeviceManagerIDTest : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(DeviceManagerIDTest);
	CPPUNIT_TEST(testCreate);
	CPPUNIT_TEST(testCreateFromParts);
	CPPUNIT_TEST(testParse);
	CPPUNIT_TEST(testInvalidCreate);
CPPUNIT_TEST_SUITE_END();

public:
	void testCreate();
	void testCreateFromParts();
	void testParse();
	void testInvalidCreate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DeviceManagerIDTest);

void DeviceManagerIDTest::testCreate()
{
	DeviceManagerID deviceManagerID(0xa1aa);

	CPPUNIT_ASSERT_EQUAL(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL),
		deviceManagerID.prefix());
	CPPUNIT_ASSERT_EQUAL((uint8_t) 0xaa, deviceManagerID.ident());
	CPPUNIT_ASSERT(deviceManagerID.toString().compare("0xa1aa"));
}

void DeviceManagerIDTest::testCreateFromParts()
{
	DeviceManagerID deviceManagerID(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL),
		0x12);

	CPPUNIT_ASSERT_EQUAL(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL),
		deviceManagerID.prefix());
	CPPUNIT_ASSERT_EQUAL((uint8_t) 0x12, deviceManagerID.ident());
	CPPUNIT_ASSERT(deviceManagerID.toString().compare("0xa112"));
}

void DeviceManagerIDTest::testParse()
{
	DeviceManagerID deviceManagerID = DeviceManagerID::parse("0xa112");

	CPPUNIT_ASSERT_EQUAL(
		DevicePrefix::fromRaw(DevicePrefix::PREFIX_FITPROTOCOL),
		deviceManagerID.prefix());
	CPPUNIT_ASSERT_EQUAL((uint8_t) 0x12, deviceManagerID.ident());
	CPPUNIT_ASSERT(deviceManagerID.toString().compare("0xa112"));
}

void DeviceManagerIDTest::testInvalidCreate()
{
	// Parse with PREFIX_INVALID
	CPPUNIT_ASSERT_THROW(DeviceManagerID(0), Poco::InvalidArgumentException);

	// Parse with unrecognized raw value
	CPPUNIT_ASSERT_THROW(DeviceManagerID(0xffff), Poco::InvalidArgumentException);

	// It parses bigger numbers than uint16_t
	CPPUNIT_ASSERT_THROW(DeviceManagerID::parse("99999999"), Poco::RangeException);
}

}
