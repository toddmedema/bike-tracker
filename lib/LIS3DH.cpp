
#include "Particle.h"
#include "LIS3DH.h"

#if PLATFORM_THREADING
static Mutex syncCallbackMutex;
#else
// On the core, there is no mutex support
static volatile bool syncCallbackDone;
#endif


LIS3DH::LIS3DH(SPIClass &spi, int ss, int intPin) : spi(spi), ss(ss), intPin(intPin) {
	spi.begin(ss);

	// This initialization possibly should go in beginTransaction() for compatibility with SPI
	// bus sharing in different modes, but it seems to require an indeterminate delay to take
	// effect, otherwise the operations fail. Since SPI bus sharing across modes tends not to
	// work with other devices, anyway, I put the code here.
	spi.setBitOrder(MSBFIRST);
	spi.setClockSpeed(1, MHZ);
	spi.setDataMode(SPI_MODE0); // CPHA = 0, CPOL = 0 : MODE = 0
}

LIS3DH::~LIS3DH() {

}

bool LIS3DH::setupLowPowerWakeMode(uint8_t movementThreshold) {

	bool found = false;
	for(int tries = 0; tries < 10; tries++) {
		uint8_t whoami = readRegister8(REG_WHO_AM_I);
		if (whoami == WHO_AM_I) {
			found = true;
			break;
		}
		delay(1);
	}
	if (!found) {
		return false;
	}

	// Enable 10 Hz, low power, with XYZ detection enabled
	writeRegister8(REG_CTRL_REG1, CTRL_REG1_ODR1 | CTRL_REG1_LPEN | CTRL_REG1_ZEN | CTRL_REG1_YEN | CTRL_REG1_XEN);

	// High pass filters disabled
	// Enable reference mode CTRL_REG2_HPM0 | CTRL_REG2_HPIS1
	// Tried enabling CTRL_REG2_HPM0 | CTRL_REG2_HPM1 for auto-reset, did not seem to help
	writeRegister8(REG_CTRL_REG2, 0);

	// Enable INT1
	writeRegister8(REG_CTRL_REG3, CTRL_REG3_I1_INT1);

	// Disable high resolution mode
	writeRegister8(REG_CTRL_REG4, 0);

	// Page 12 of the app note says to do this last, but page 25 says to do them in order.
	// Disable FIFO, enable latch interrupt on INT1_SRC
	writeRegister8(REG_CTRL_REG5, CTRL_REG5_LIR_INT1);

	// CTRL_REG6_H_LACTIVE means active low, not needed here
	writeRegister8(REG_CTRL_REG6, 0);

	// In normal mode, reading the reference register sets it for the current normal force
	// (the normal force of gravity acting on the device)
	readRegister8(REG_REFERENCE);

	// 250 mg threshold = 16
	writeRegister8(REG_INT1_THS, movementThreshold);

	//
	writeRegister8(REG_INT1_DURATION, 0);


	if (intPin >= 0) {
		// There are instructions to set the INT1_CFG in a loop in the appnote on page 24. As far
		// as I can tell this never works. Merely setting the INT1_CFG does not ever generate an
		// interrupt for me.

		// Remember the INT1_CFG setting because we're apparently supposed to set it again after
		// clearing an interrupt.
		int1_cfg = INT1_CFG_YHIE_YUPE | INT1_CFG_XHIE_XUPE;
		writeRegister8(REG_INT1_CFG, int1_cfg);

		// Clear the interrupt just in case
		readRegister8(REG_INT1_SRC);
	}
	else {
		int1_cfg = 0;
		writeRegister8(REG_INT1_CFG, 0);
	}

	return true;
}

uint8_t LIS3DH::clearInterrupt() {
	uint8_t int1_src = readRegister8(REG_INT1_SRC);

	if (intPin >= 0) {
		while(digitalRead(intPin) == HIGH) {
			delay(10);
			readRegister8(REG_INT1_SRC);
			writeRegister8(REG_INT1_CFG, int1_cfg);
		}
	}

	return int1_src;
}

void LIS3DH::enableTemperature(boolean enable) {
	writeRegister8(REG_TEMP_CFG_REG, enable ? (TEMP_CFG_TEMP_EN | TEMP_CFG_ADC_PD) : 0);
}


int16_t LIS3DH::getTemperature() {

	// https://my.st.com/public/STe2ecommunities/mcu/Lists/STM32F%20MEMS%20%20iNEMO/flat.aspx?RootFolder=%2Fpublic%2FSTe2ecommunities%2Fmcu%2FLists%2FSTM32F%20MEMS%20%20iNEMO%2FLIS3DH%20temperature%20sensor&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580003E26E7DD54228C428E8F9FB5EE9C5185&currentviews=2886

	int16_t result = ((int16_t) readRegister16(REG_OUT_ADC3_L)) / 256;

	return result;
}

uint8_t LIS3DH::readRegister8(uint8_t addr) {
	uint8_t req[2], resp[2];

	req[0] = SPI_READ | addr;
	req[1] = 0;

	syncTransaction(req, resp, sizeof(req));

	return resp[1];
}

uint16_t LIS3DH::readRegister16(uint8_t addr) {
	uint8_t req[3], resp[3];

	req[0] = SPI_READ | SPI_INCREMENT | addr;
	req[1] = req[2] = 0;

	syncTransaction(req, resp, sizeof(req));

	return resp[1] | (((uint16_t)resp[2]) << 8);
}


void LIS3DH::writeRegister8(uint8_t addr, uint8_t value) {
	// Serial.printlnf("writeRegister addr=%02x value=%02x", addr, value);

	uint8_t req[2], resp[2];

	req[0] = addr;
	req[1] = value;

	syncTransaction(req, resp, sizeof(req));
}

void LIS3DH::writeRegister16(uint8_t addr, uint16_t value) {
	// Serial.printlnf("writeRegister addr=%02x value=%04x", addr, value);

	uint8_t req[3], resp[3];

	req[0] = SPI_INCREMENT | addr;
	req[1] = value & 0xff;
	req[2] = value >> 8;

	syncTransaction(req, resp, sizeof(req));
}



void LIS3DH::beginTransaction() {
	// See note in the constructor for LIS3DH
	busy = true;
	digitalWrite(ss, LOW);
}

void LIS3DH::endTransaction() {
	digitalWrite(ss, HIGH);
	busy = false;
}

void LIS3DH::syncTransaction(void *req, void *resp, size_t len) {
#if PLATFORM_THREADING
	syncCallbackMutex.lock();

	beginTransaction();

	spi.transfer(req, resp, len, syncCallback);
	syncCallbackMutex.lock();

	endTransaction();

	syncCallbackMutex.unlock();
#else
	syncCallbackDone = false;
	beginTransaction();

	spi.transfer(req, resp, len, syncCallback);

	while(!syncCallbackDone) {
	}

	endTransaction();
#endif
}

// [static]
void LIS3DH::syncCallback(void) {
#if PLATFORM_THREADING
	syncCallbackMutex.unlock();
#else
	syncCallbackDone = true;
#endif
}
