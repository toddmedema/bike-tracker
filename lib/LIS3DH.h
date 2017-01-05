#ifndef __LIS3DH_H
#define __LIS3DH_H

// Interface to the LIS30DH accelerometer used on the Particle AssetTracker for Electron
// This is similar to the API for the ADXL362DMA driver I wrote for that accelerometer

// Note: The actual FIFO interface is not implemented yet, because I didn't need it
// for my current project. Only the intertial wake is supported, which is probably what
// most people will use this for.

// Data sheet:
// http://www.st.com/resource/en/datasheet/lis3dh.pdf
//
// Extremely helpful application note:
// http://www.st.com/resource/en/application_note/cd00290365.pdf

class LIS3DH {
public:
	LIS3DH(SPIClass &spi, int ss = A2, int intPin = -1);
	virtual ~LIS3DH();

	/**
	 * Initializes the device in low power wake on movement mode
	 */
	bool setupLowPowerWakeMode(uint8_t movementThreshold = 16);


	/**
	 * After getting an interrupt, call this to read the interrupt status and clear the interrupt
	 * The resulting value is the value of the INT1_SRC register.
	 */
	uint8_t clearInterrupt();

	/**
	 * Enables the temperature sensor. Call once, usually when you call setupLowPowerWakeMode.
	 */
	void enableTemperature(boolean enable = true);

	/**
	 * Gets the temperature of the sensor in degrees C
	 *
	 * Make sure you call enableTemperature() at when you're setting the modes, because it seems
	 * to take a while to start up, you can't just keep turning it on and off all the time,
	 * apparently.
	 */
	int16_t getTemperature();

	/**
	 * Reads an 8-bit register value
	 *
	 * Most of the calls have easier to use accessors like readStatus() that use this call internally.
	 *
	 * addr: One of the register addresses, such as REG_STATUS
	 */
	uint8_t readRegister8(uint8_t addr);

	/**
	 * Reads an 16-bit register value
	 *
	 * addr: One of the register addresses, such as REG_THRESH_ACT_L. It must be the first of a pair of _L and _H registers.
	 */
	uint16_t readRegister16(uint8_t addr);

	/**
	 * Write an 8-bit register value
	 *
	 * Most of the calls have easier to use accessors like writeIntmap1() that use this call internally.
	 *
	 * addr: One of the register addresses, such as REG_INTMAP1
	 */
	void writeRegister8(uint8_t addr, uint8_t value);

	/**
	 * Write an 16-bit register value
	 *
	 * Most of the calls have easier to use accessors like writeIntmap1() that use this call internally.
	 *
	 * addr: One of the register addresses, such as REG_THRESH_ACT_L. It must be the first of a pair of _L and _H registers.
	 */
	void writeRegister16(uint8_t addr, uint16_t value);

	/**
	 * Returns true if a SPI command is currently being handled
	 */
	bool getIsBusy() { return busy; };

	/**
	 * Begin a synchronous SPI DMI transaction
	 */
	void syncTransaction(void *req, void *resp, size_t len);


	static const uint8_t SPI_READ = 0x80;
	static const uint8_t SPI_INCREMENT= 0x40;

	static const uint8_t WHO_AM_I = 0b00110011;

	static const uint8_t REG_STATUS_AUX = 0x07;
	static const uint8_t REG_OUT_ADC1_L = 0x08;
	static const uint8_t REG_OUT_ADC1_H = 0x09;
	static const uint8_t REG_OUT_ADC2_L = 0x0a;
	static const uint8_t REG_OUT_ADC2_H = 0x0b;
	static const uint8_t REG_OUT_ADC3_L = 0x0c;
	static const uint8_t REG_OUT_ADC3_H = 0x0d;
	static const uint8_t REG_INT_COUNTER_REG = 0x0e;
	static const uint8_t REG_WHO_AM_I = 0x0f;
	static const uint8_t REG_TEMP_CFG_REG = 0x1f;
	static const uint8_t REG_CTRL_REG1 = 0x20;
	static const uint8_t REG_CTRL_REG2 = 0x21;
	static const uint8_t REG_CTRL_REG3 = 0x22;
	static const uint8_t REG_CTRL_REG4 = 0x23;
	static const uint8_t REG_CTRL_REG5 = 0x24;
	static const uint8_t REG_CTRL_REG6 = 0x25;
	static const uint8_t REG_REFERENCE = 0x26;
	static const uint8_t REG_STATUS_REG = 0x27;
	static const uint8_t REG_OUT_X_L = 0x28;
	static const uint8_t REG_OUT_X_H = 0x29;
	static const uint8_t REG_OUT_Y_L = 0x2a;
	static const uint8_t REG_OUT_Y_H = 0x2b;
	static const uint8_t REG_OUT_Z_L = 0x2c;
	static const uint8_t REG_OUT_Z_H = 0x2d;
	static const uint8_t REG_FIFO_CTRL_REG = 0x2e;
	static const uint8_t REG_FIFO_SRC_REG = 0x2f;
	static const uint8_t REG_INT1_CFG = 0x30;
	static const uint8_t REG_INT1_SRC = 0x31;
	static const uint8_t REG_INT1_THS = 0x32;
	static const uint8_t REG_INT1_DURATION = 0x33;
	static const uint8_t REG_CLICK_CFG = 0x38;
	static const uint8_t REG_CLICK_SRC = 0x39;
	static const uint8_t REG_CLICK_THS = 0x3a;
	static const uint8_t REG_TIME_LIMIT = 0x3b;
	static const uint8_t REG_TIME_LATENCY = 0x3c;
	static const uint8_t REG_TIME_WINDOW = 0x3d;

	static const uint8_t STATUS_AUX_321OR = 0x80;
	static const uint8_t STATUS_AUX_3OR = 0x40;
	static const uint8_t STATUS_AUX_2OR = 0x20;
	static const uint8_t STATUS_AUX_1OR = 0x10;
	static const uint8_t STATUS_AUX_321DA = 0x08;
	static const uint8_t STATUS_AUX_3DA = 0x04;
	static const uint8_t STATUS_AUX_2DA = 0x02;
	static const uint8_t STATUS_AUX_1DA = 0x01;

	static const uint8_t CTRL_REG1_ODR3 = 0x80;
	static const uint8_t CTRL_REG1_ODR2 = 0x40;
	static const uint8_t CTRL_REG1_ODR1 = 0x20;
	static const uint8_t CTRL_REG1_ODR0 = 0x10;
	static const uint8_t CTRL_REG1_LPEN = 0x08;
	static const uint8_t CTRL_REG1_ZEN = 0x04;
	static const uint8_t CTRL_REG1_YEN = 0x02;
	static const uint8_t CTRL_REG1_XEN = 0x01;

	static const uint8_t CTRL_REG2_HPM1 = 0x80;
	static const uint8_t CTRL_REG2_HPM0 = 0x40;
	static const uint8_t CTRL_REG2_HPCF2 = 0x20;
	static const uint8_t CTRL_REG2_HPCF1 = 0x10;
	static const uint8_t CTRL_REG2_FDS = 0x08;
	static const uint8_t CTRL_REG2_HPCLICK = 0x04;
	static const uint8_t CTRL_REG2_HPIS2 = 0x02;
	static const uint8_t CTRL_REG2_HPIS1 = 0x01;


	static const uint8_t CTRL_REG3_I1_CLICK = 0x80;
	static const uint8_t CTRL_REG3_I1_INT1 = 0x40;
	static const uint8_t CTRL_REG3_I1_DRDY = 0x10;
	static const uint8_t CTRL_REG3_I1_WTM = 0x04;
	static const uint8_t CTRL_REG3_I1_OVERRUN = 0x02;

	static const uint8_t CTRL_REG4_BDU = 0x80;
	static const uint8_t CTRL_REG4_BLE = 0x40;
	static const uint8_t CTRL_REG4_FS1 = 0x20;
	static const uint8_t CTRL_REG4_FS0 = 0x10;
	static const uint8_t CTRL_REG4_HR = 0x08;
	static const uint8_t CTRL_REG4_ST1 = 0x04;
	static const uint8_t CTRL_REG4_ST0 = 0x02;
	static const uint8_t CTRL_REG4_SIM = 0x01;


	static const uint8_t CTRL_REG5_BOOT = 0x80;
	static const uint8_t CTRL_REG5_FIFO_EN = 0x40;
	static const uint8_t CTRL_REG5_LIR_INT1 = 0x08;
	static const uint8_t CTRL_REG5_D4D_INT1 = 0x04;

	static const uint8_t CTRL_REG6_I2_CLICK = 0x80;
	static const uint8_t CTRL_REG6_I2_INT2 = 0x40;
	static const uint8_t CTRL_REG6_BOOT_I2 = 0x10;
	static const uint8_t CTRL_REG6_H_LACTIVE = 0x02;

	static const uint8_t INT1_CFG_AOI = 0x80;
	static const uint8_t INT1_CFG_6D = 0x40;
	static const uint8_t INT1_CFG_ZHIE_ZUPE = 0x20;
	static const uint8_t INT1_CFG_ZLIE_ZDOWNE = 0x10;
	static const uint8_t INT1_CFG_YHIE_YUPE = 0x08;
	static const uint8_t INT1_CFG_YLIE_YDOWNE = 0x04;
	static const uint8_t INT1_CFG_XHIE_XUPE = 0x02;
	static const uint8_t INT1_CFG_XLIE_XDOWNE = 0x01;

	static const uint8_t INT1_SRC_IA = 0x40;
	static const uint8_t INT1_SRC_ZH = 0x20;
	static const uint8_t INT1_SRC_ZL = 0x10;
	static const uint8_t INT1_SRC_YH = 0x08;
	static const uint8_t INT1_SRC_YL = 0x04;
	static const uint8_t INT1_SRC_XH = 0x02;
	static const uint8_t INT1_SRC_XL = 0x01;

	static const uint8_t TEMP_CFG_ADC_PD = 0x80;
	static const uint8_t TEMP_CFG_TEMP_EN = 0x40;

private:

	void beginTransaction();
	void endTransaction();

	static void syncCallback(void);

	SPIClass &spi; // Typically SPI or SPI1
	int ss;		// SS or /CS chip select pin. Default: A2
	int intPin; // Pin connected to INT1 on the accelerometer (-1 = not connected)
	bool busy = false;
	uint8_t int1_cfg; // What we set as INT1_CFG
};

#endif /* __LIS30DH_H */
