#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "VL6180X_I2C.h"
#include "sdkconfig.h"

// RANGE_SCALER values for 1x, 2x, 3x scaling - see STSW-IMG003 core/src/vl6180x_api.c (ScalerLookUP[])
static uint16_t const ScalerValues[] = {0, 253, 127, 84};

// Writes an 8-bit register
void VL6180X_writeReg(uint8_t address, uint16_t reg, uint8_t value)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
   i2c_master_write_byte(cmd, ((reg >> 8) & 0xFF), 1);
   i2c_master_write_byte(cmd, (reg & 0xFF), 1);
   i2c_master_write_byte(cmd, value, 1);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
}

// Writes a 16-bit register
void VL6180X_writeReg16Bit(uint8_t address, uint16_t reg, uint16_t value)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
   i2c_master_write_byte(cmd, ((reg >> 8) & 0xFF), 1);
   i2c_master_write_byte(cmd, (reg & 0xFF), 1);
   i2c_master_write_byte(cmd, ((value >> 8) & 0xFF), 1);
   i2c_master_write_byte(cmd, (value & 0xFF), 1);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
}

// Writes a 32-bit register
void VL6180X_writeReg32Bit(uint8_t address, uint16_t reg, uint32_t value)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
   i2c_master_write_byte(cmd, ((reg >> 8) & 0xFF), 1);
   i2c_master_write_byte(cmd, (reg & 0xFF), 1);
   i2c_master_write_byte(cmd, ((value >> 24) & 0xFF), 1);
   i2c_master_write_byte(cmd, ((value >> 16) & 0xFF), 1);
   i2c_master_write_byte(cmd, ((value >> 8) & 0xFF), 1);
   i2c_master_write_byte(cmd, (value & 0xFF), 1);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
}

// Reads an 8-bit register
uint8_t VL6180X_readReg(uint8_t address, uint16_t reg)
{
  uint8_t value;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
  i2c_master_write_byte(cmd, ((reg >> 8) & 0xFF), 1);
  i2c_master_write_byte(cmd, (reg & 0xFF), 1);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, 1 /* expect ack */);
  i2c_master_read_byte(cmd, &value, 1);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return value;
}

// Reads a 16-bit register
uint16_t VL6180X_readReg16Bit(uint8_t address, uint16_t reg)
{
  uint16_t value;
  uint8_t val_h;
  uint8_t val_l;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
  i2c_master_write_byte(cmd, ((reg >> 8) & 0xFF), 1);
  i2c_master_write_byte(cmd, (reg & 0xFF), 1);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, 1 /* expect ack */);
  i2c_master_read_byte(cmd, &val_h, 0);
  i2c_master_read_byte(cmd, &val_l, 0);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  value = val_h << 8 | val_l;

  return value;
}

// Reads a 32-bit register
uint32_t VL6180X_readReg32Bit(uint8_t address, uint16_t reg)
{
  uint32_t value;
  uint8_t val_32;
  uint8_t val_24;
  uint8_t val_16;
  uint8_t val_8;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
  i2c_master_write_byte(cmd, ((reg >> 8) & 0xFF), 1);
  i2c_master_write_byte(cmd, (reg & 0xFF), 1);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, 1 /* expect ack */);
  i2c_master_read_byte(cmd, &val_32, 1);
  i2c_master_read_byte(cmd, &val_24, 1);
  i2c_master_read_byte(cmd, &val_16, 1);
  i2c_master_read_byte(cmd, &val_8, 1);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_NUM_0, cmd, 200 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  value = val_32 << 24 | val_24 << 16 | val_16 << 8 | val_8;
  return value;
}

void VL6180X_setAddress(struct VL6180X_data* dev, uint8_t new_addr)
{
  VL6180X_writeReg(dev->i2c_address, I2C_SLAVE_DEVICE_ADDRESS, new_addr & 0x7F);
  dev->i2c_address = new_addr;
  //printf("VL6180X_SetAddress New: 0x%.2X Dev: 0x%.2X\n", new_addr, dev->i2c_address);
}

// Initialize sensor with settings from ST application note AN4545, section 9 -
// "Mandatory : private registers"
void VL6180X_init(struct VL6180X_data* dev)
{
	// Store part-to-part range offset so it can be adjusted if scaling is changed
	dev->ptp_offset = VL6180X_readReg(dev->i2c_address, SYSRANGE_PART_TO_PART_RANGE_OFFSET);

	if (VL6180X_readReg(dev->i2c_address, SYSTEM_FRESH_OUT_OF_RESET) == 1)
	{
		dev->scaling = 1;

		VL6180X_writeReg(dev->i2c_address, 0x207, 0x01);
		VL6180X_writeReg(dev->i2c_address, 0x208, 0x01);
		VL6180X_writeReg(dev->i2c_address, 0x096, 0x00);
		VL6180X_writeReg(dev->i2c_address, 0x097, 0xFD); // RANGE_SCALER = 253
		VL6180X_writeReg(dev->i2c_address, 0x0E3, 0x00);
		VL6180X_writeReg(dev->i2c_address, 0x0E4, 0x04);
		VL6180X_writeReg(dev->i2c_address, 0x0E5, 0x02);
		VL6180X_writeReg(dev->i2c_address, 0x0E6, 0x01);
		VL6180X_writeReg(dev->i2c_address, 0x0E7, 0x03);
		VL6180X_writeReg(dev->i2c_address, 0x0F5, 0x02);
		VL6180X_writeReg(dev->i2c_address, 0x0D9, 0x05);
		VL6180X_writeReg(dev->i2c_address, 0x0DB, 0xCE);
		VL6180X_writeReg(dev->i2c_address, 0x0DC, 0x03);
		VL6180X_writeReg(dev->i2c_address, 0x0DD, 0xF8);
		VL6180X_writeReg(dev->i2c_address, 0x09F, 0x00);
		VL6180X_writeReg(dev->i2c_address, 0x0A3, 0x3C);
		VL6180X_writeReg(dev->i2c_address, 0x0B7, 0x00);
		VL6180X_writeReg(dev->i2c_address, 0x0BB, 0x3C);
		VL6180X_writeReg(dev->i2c_address, 0x0B2, 0x09);
		VL6180X_writeReg(dev->i2c_address, 0x0CA, 0x09);
		VL6180X_writeReg(dev->i2c_address, 0x198, 0x01);
		VL6180X_writeReg(dev->i2c_address, 0x1B0, 0x17);
		VL6180X_writeReg(dev->i2c_address, 0x1AD, 0x00);
		VL6180X_writeReg(dev->i2c_address, 0x0FF, 0x05);
		VL6180X_writeReg(dev->i2c_address, 0x100, 0x05);
		VL6180X_writeReg(dev->i2c_address, 0x199, 0x05);
		VL6180X_writeReg(dev->i2c_address, 0x1A6, 0x1B);
		VL6180X_writeReg(dev->i2c_address, 0x1AC, 0x3E);
		VL6180X_writeReg(dev->i2c_address, 0x1A7, 0x1F);
		VL6180X_writeReg(dev->i2c_address, 0x030, 0x00);

		VL6180X_writeReg(dev->i2c_address, SYSTEM_FRESH_OUT_OF_RESET, 0);
	}
	else
	{
		// Sensor has already been initialized, so try to get scaling settings by
		// reading registers.

		uint16_t s = VL6180X_readReg16Bit(dev->i2c_address, RANGE_SCALER);

		if      (s == ScalerValues[3]) { dev->scaling = 3; }
		else if (s == ScalerValues[2]) { dev->scaling = 2; }
		else                           { dev->scaling = 1; }

		// Adjust the part-to-part range offset value read earlier to account for
		// existing scaling. If the sensor was already in 2x or 3x scaling mode,
		// precision will be lost calculating the original (1x) offset, but this can
		// be resolved by resetting the sensor and Arduino again.
		dev->ptp_offset *= dev->scaling;
	}
}

// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// writeReg(SYSTEM__MODE_GPIO1, 0x10);
void VL6180X_configureDefault(struct VL6180X_data* dev)
{
  // "Recommended : Public registers"
	// GPIO0
	VL6180X_writeReg(dev->i2c_address, SYSTEM_MODE_GPIO0, 0x60);

	// GPIO1
	VL6180X_writeReg(dev->i2c_address, SYSTEM_MODE_GPIO1, 0x10);

	// readout__averaging_sample_period = 48
	VL6180X_writeReg(dev->i2c_address, READOUT_AVERAGING_SAMPLE_PERIOD, 0x30);

	// sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01 according to Table 14 in datasheet)
	VL6180X_writeReg(dev->i2c_address, SYSALS_ANALOGUE_GAIN, 0x46);

	// sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature recalibration after every 255 range measurements)
	VL6180X_writeReg(dev->i2c_address, SYSRANGE_VHV_REPEAT_RATE, 0xFF);

	// sysals__integration_period = 99 (100 ms)
	// AN4545 incorrectly recommends writing to register 0x040; 0x63 should go in the lower byte, which is register 0x041.
	VL6180X_writeReg16Bit(dev->i2c_address, SYSALS_INTEGRATION_PERIOD, 0x0063);

	// sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)
	VL6180X_writeReg(dev->i2c_address, SYSRANGE_VHV_RECALIBRATE, 0x01);

	// "Optional: Public registers"

	// sysrange__intermeasurement_period = 9 (100 ms)
	VL6180X_writeReg(dev->i2c_address, SYSRANGE_INTERMEASUREMENT_PERIOD, 0x09);

	// sysals__intermeasurement_period = 49 (500 ms)
	VL6180X_writeReg(dev->i2c_address, SYSALS_INTERMEASUREMENT_PERIOD, 0x31);

	// als_int_mode = 4 (ALS new sample ready interrupt); range_int_mode = 4 (range new sample ready interrupt)
	VL6180X_writeReg(dev->i2c_address, SYSTEM_INTERRUPT_CONFIG_GPIO, 0x24);

	// Reset other settings to power-on defaults

	// sysrange__max_convergence_time = 49 (49 ms)
	VL6180X_writeReg(dev->i2c_address, SYSRANGE_MAX_CONVERGENCE_TIME, 0x31);

	// disable interleaved mode
	VL6180X_writeReg(dev->i2c_address, INTERLEAVED_MODE_ENABLE, 0);

	// reset range scaling factor to 1x and Timeout 500
	VL6180X_setScaling(dev, 1, 500);
}

// Set range scaling factor. The sensor uses 1x scaling by default, giving range
// measurements in units of mm. Increasing the scaling to 2x or 3x makes it give
// raw values in units of 2 mm or 3 mm instead. In other words, a bigger scaling
// factor increases the sensor's potential maximum range but reduces its
// resolution.

// Implemented using ST's VL6180X API as a reference (STSW-IMG003); see
// VL6180x_UpscaleSetScaling() in vl6180x_api.c.
void VL6180X_setScaling(struct VL6180X_data* dev, uint8_t new_scaling, uint16_t timeout)
{
  uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

  // do nothing if scaling value is invalid
  if (new_scaling < 1 || new_scaling > 3) { return; }

  dev->scaling = new_scaling;
  dev->io_timeout = timeout;
  VL6180X_writeReg16Bit(dev->i2c_address, RANGE_SCALER, ScalerValues[dev->scaling]);

  // apply scaling on part-to-part offset
  VL6180X_writeReg(dev->i2c_address, SYSRANGE_PART_TO_PART_RANGE_OFFSET, dev->ptp_offset / dev->scaling);

  // apply scaling on CrossTalkValidHeight
  VL6180X_writeReg(dev->i2c_address, SYSRANGE_CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / dev->scaling);

  // This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

  // enable early convergence estimate only at 1x scaling
  uint8_t rce = VL6180X_readReg(dev->i2c_address, SYSRANGE_RANGE_CHECK_ENABLES);
  VL6180X_writeReg(dev->i2c_address, SYSRANGE_RANGE_CHECK_ENABLES, (rce & 0xFE) | (dev->scaling == 1));
}

// Performs a single-shot ranging measurement
uint8_t VL6180X_readRangeSingle(struct VL6180X_data* dev)
{
  VL6180X_writeReg(dev->i2c_address, SYSRANGE_START, 0x01);
  return VL6180X_readRangeContinuous(dev);
}

// Performs a single-shot ambient light measurement
uint16_t VL6180X_readAmbientSingle(struct VL6180X_data* dev)
{
  VL6180X_writeReg(dev->i2c_address, SYSALS_START, 0x01);
  return VL6180X_readAmbientContinuous(dev);
}

// Returns a range reading when continuous mode is activated
// (readRangeSingle() also calls this function after starting a single-shot
// range measurement)
uint8_t VL6180X_readRangeContinuous(struct VL6180X_data* dev)
{
  uint32_t millis_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
  while ((VL6180X_readReg(dev->i2c_address, RESULT_INTERRUPT_STATUS_GPIO) & 0x04) == 0)
  {
    if (dev->io_timeout > 0 && (((uint32_t)xTaskGetTickCount() * portTICK_PERIOD_MS) - millis_start) > dev->io_timeout)
    {
      dev->did_timeout = true;
      return 255;
    }
  }

  uint8_t range = VL6180X_readReg(dev->i2c_address, RESULT_RANGE_VAL);
  VL6180X_writeReg(dev->i2c_address, SYSTEM_INTERRUPT_CLEAR, 0x01);

  return range;
}

// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
uint16_t VL6180X_readAmbientContinuous(struct VL6180X_data* dev)
{
  uint32_t millis_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
  while ((VL6180X_readReg(dev->i2c_address, RESULT_INTERRUPT_STATUS_GPIO) & 0x20) == 0)
  {
    if (dev->io_timeout > 0 && (((uint32_t)xTaskGetTickCount() * portTICK_PERIOD_MS) - millis_start) > dev->io_timeout)
    {
      dev->did_timeout = true;
      return 0;
    }
  }

  uint16_t ambient = VL6180X_readReg16Bit(dev->i2c_address, RESULT_ALS_VAL);
  VL6180X_writeReg(dev->i2c_address, SYSTEM_INTERRUPT_CLEAR, 0x02);

  return ambient;
}

uint16_t readRangeSingleMillimeters(struct VL6180X_data* dev)
{
	return (uint16_t)dev->scaling * VL6180X_readRangeSingle(dev);
}

uint16_t readRangeContinuousMillimeters(struct VL6180X_data* dev)
{
	return (uint16_t)dev->scaling * VL6180X_readRangeContinuous(dev);
}

uint8_t getScaling(struct VL6180X_data* dev)
{
	return dev->scaling;
}

uint16_t getTimeout(struct VL6180X_data* dev)
{
	return dev->io_timeout;
}

// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void VL6180X_startRangeContinuous(struct VL6180X_data* dev, uint16_t period)
{
  uint8_t period_reg = (uint8_t)(period / 10) - 1;
  if(period_reg < 10) period_reg = 10;

  VL6180X_writeReg(dev->i2c_address, SYSRANGE_INTERMEASUREMENT_PERIOD, period_reg);
  VL6180X_writeReg(dev->i2c_address, SYSRANGE_START, 0x03);
}

// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void VL6180X_startAmbientContinuous(struct VL6180X_data* dev, uint16_t period)
{
  uint8_t period_reg = (uint8_t)(period / 10) - 1;
  if(period_reg < 10) period_reg = 10;

  VL6180X_writeReg(dev->i2c_address, SYSALS_INTERMEASUREMENT_PERIOD, period_reg);
  VL6180X_writeReg(dev->i2c_address, SYSALS_START, 0x03);
}

// Starts continuous interleaved measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified). In this mode, each
// ambient light measurement is immediately followed by a range measurement.
//
// The datasheet recommends using this mode instead of running "range and ALS
// continuous modes simultaneously (i.e. asynchronously)".
//
// The period must be greater than the time it takes to perform both
// measurements. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void VL6180X_startInterleavedContinuous(struct VL6180X_data* dev, uint16_t period)
{
  uint8_t period_reg = (int8_t)(period / 10) - 1;
  if(period_reg < 10) period_reg = 10;

  VL6180X_writeReg(dev->i2c_address, INTERLEAVED_MODE_ENABLE, 1);
  VL6180X_writeReg(dev->i2c_address, SYSALS_INTERMEASUREMENT_PERIOD, period_reg);
  VL6180X_writeReg(dev->i2c_address, SYSALS_START, 0x03);
}

// Stops continuous mode. This will actually start a single measurement of range
// and/or ambient light if continuous mode is not active, so it's a good idea to
// wait a few hundred ms after calling this function to let that complete
// before starting continuous mode again or taking a reading.
void VL6180X_stopContinuous(struct VL6180X_data* dev)
{

  VL6180X_writeReg(dev->i2c_address, SYSRANGE_START, 0x01);
  VL6180X_writeReg(dev->i2c_address, SYSALS_START, 0x01);

  VL6180X_writeReg(dev->i2c_address, INTERLEAVED_MODE_ENABLE, 0);
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool VL6180X_timeoutOccurred(struct VL6180X_data* dev)
{
  bool tmp = dev->did_timeout;
  dev->did_timeout = false;
  return tmp;
}

uint8_t VL6180X_getrangestatus(struct VL6180X_data* dev)
{
	uint8_t val;
	val = (VL6180X_readReg(dev->i2c_address, RESULT_RANGE_STATUS) >> 4);
	return (val & 0x0F);
}

uint8_t VL6180X_getinterruptstatus(struct VL6180X_data* dev)
{
	uint8_t val;
	uint8_t error;

	val = VL6180X_readReg(dev->i2c_address, RESULT_INTERRUPT_STATUS_GPIO);
	val = val & 0xC7;
	error = val & 0xC0;
	switch(error) {
		case 0:
			break;
		case 1:
			printf("Erro Laser Safety\n");
			break;
		case 2:
			printf("Erro PLL\n");
			break;
	}
	error = val & 0x07;
	return error;
}

void VL6180X_setrangewindow(struct VL6180X_data* dev, uint8_t max, uint8_t min)
{
	VL6180X_writeReg(dev->i2c_address, SYSRANGE_THRESH_HIGH, max);
	VL6180X_writeReg(dev->i2c_address, SYSRANGE_THRESH_LOW, min);
}
