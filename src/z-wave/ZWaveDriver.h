#pragma once

#include <string>

#include <Poco/RegularExpression.h>

#include <Manager.h>

namespace BeeeOn {

/*
 * The Driver class handles communication between OpenZWave and
 * a device attached via a serial port (typically a controller).
 * Class requires the name of the USB device to be opened.
 * If the requested driver cannot be loaded program is terminated.
 */
class ZWaveDriver {
public:
	ZWaveDriver(std::string driver):
		m_driver(driver)
	{
	}

	/*
	 * Get USB driver path
	 * @return USB driver path
	 */
	std::string getUSBDriverPath()
	{
		return m_driver;
	}

	/*
	 * Check it is usb device.
	 * @return true if it is usb device
	 */
	bool isUSBDriver() const
	{
		Poco::RegularExpression re("usb");

		if (re.match(m_driver))
			return true;

		return false;
	}

	/*
	 * Address a new driver for a Z-Wave controller.
	 * @return True if driver successfully added
	 */
	bool registerItself() {
		if (isUSBDriver())
			return OpenZWave::Manager::Get()->AddDriver("HID Controller",
				OpenZWave::Driver::ControllerInterface_Hid);
		else
			return OpenZWave::Manager::Get()->AddDriver(m_driver);
	}

	/*
	 * Removes the driver for a Z-Wave controller.
	 * and closes the controller.
	 * @return True if the driver was removed, false if it could not be found
	 */
	bool unregisterItself() {
		if (isUSBDriver())
			return OpenZWave::Manager::Get()->RemoveDriver("HID Controller");
		else
			return OpenZWave::Manager::Get()->RemoveDriver(m_driver);
	}

private:
	std::string m_driver;
};

}
