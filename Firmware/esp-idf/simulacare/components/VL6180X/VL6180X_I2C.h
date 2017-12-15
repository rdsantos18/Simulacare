#define VL6180x_FAILURE_RESET  -1

#define IDENTIFICATION_MODEL_ID              0x0000
#define IDENTIFICATION_MODEL_REV_MAJOR       0x0001
#define IDENTIFICATION_MODEL_REV_MINOR       0x0002
#define IDENTIFICATION_MODULE_REV_MAJOR      0x0003
#define IDENTIFICATION_MODULE_REV_MINOR      0x0004
#define IDENTIFICATION_DATE                  0x0006 //16bit value
#define IDENTIFICATION_TIME                  0x0008 //16bit value

#define SYSTEM_MODE_GPIO0                    0x0010
#define SYSTEM_MODE_GPIO1                    0x0011
#define SYSTEM_HISTORY_CTRL                  0x0012
#define SYSTEM_INTERRUPT_CONFIG_GPIO         0x0014
#define SYSTEM_INTERRUPT_CLEAR               0x0015
#define SYSTEM_FRESH_OUT_OF_RESET            0x0016
#define SYSTEM_GROUPED_PARAMETER_HOLD        0x0017

#define SYSRANGE_START                       0x0018
#define SYSRANGE_THRESH_HIGH                 0x0019
#define SYSRANGE_THRESH_LOW                  0x001A
#define SYSRANGE_INTERMEASUREMENT_PERIOD     0x001B
#define SYSRANGE_MAX_CONVERGENCE_TIME        0x001C
#define SYSRANGE_CROSSTALK_COMPENSATION_RATE 0x001E
#define SYSRANGE_CROSSTALK_VALID_HEIGHT      0x0021
#define SYSRANGE_EARLY_CONVERGENCE_ESTIMATE  0x0022
#define SYSRANGE_PART_TO_PART_RANGE_OFFSET   0x0024
#define SYSRANGE_RANGE_IGNORE_VALID_HEIGHT   0x0025
#define SYSRANGE_RANGE_IGNORE_THRESHOLD      0x0026
#define SYSRANGE_MAX_AMBIENT_LEVEL_MULT      0x002C
#define SYSRANGE_RANGE_CHECK_ENABLES         0x002D
#define SYSRANGE_VHV_RECALIBRATE             0x002E
#define SYSRANGE_VHV_REPEAT_RATE             0x0031

#define SYSALS_START                         0x0038
#define SYSALS_THRESH_HIGH                   0x003A
#define SYSALS_THRESH_LOW                    0x003C
#define SYSALS_INTERMEASUREMENT_PERIOD       0x003E
#define SYSALS_ANALOGUE_GAIN                 0x003F
#define SYSALS_INTEGRATION_PERIOD            0x0040

#define RESULT_RANGE_STATUS                  0x004D
#define RESULT_ALS_STATUS                    0x004E
#define RESULT_INTERRUPT_STATUS_GPIO         0x004F
#define RESULT_ALS_VAL                       0x0050
#define RESULT_HISTORY_BUFFER                0x0052
#define RESULT_RANGE_VAL                     0x0062
#define RESULT_RANGE_RAW                     0x0064
#define RESULT_RANGE_RETURN_RATE             0x0066
#define RESULT_RANGE_REFERENCE_RATE          0x0068
#define RESULT_RANGE_RETURN_SIGNAL_COUNT     0x006C
#define RESULT_RANGE_REFERENCE_SIGNAL_COUNT  0x0070
#define RESULT_RANGE_RETURN_AMB_COUNT        0x0074
#define RESULT_RANGE_REFERENCE_AMB_COUNT     0x0078
#define RESULT_RANGE_RETURN_CONV_TIME        0x007C
#define RESULT_RANGE_REFERENCE_CONV_TIME     0x0080
#define RANGE_SCALER					     0x0096

#define READOUT_AVERAGING_SAMPLE_PERIOD      0x010A
#define FIRMWARE_BOOTUP                      0x0119
#define FIRMWARE_RESULT_SCALER               0x0120
#define I2C_SLAVE_DEVICE_ADDRESS             0x0212
#define INTERLEAVED_MODE_ENABLE              0x02A3

#define VL6180X_ERROR_NONE         0
#define VL6180X_ERROR_SYSERR_1     1
#define VL6180X_ERROR_SYSERR_5     5
#define VL6180X_ERROR_ECEFAIL      6
#define VL6180X_ERROR_NOCONVERGE   7
#define VL6180X_ERROR_RANGEIGNORE  8
#define VL6180X_ERROR_SNR          11
#define VL6180X_ERROR_RAWUFLOW     12
#define VL6180X_ERROR_RAWOFLOW     13
#define VL6180X_ERROR_RANGEUFLOW   14
#define VL6180X_ERROR_RANGEOFLOW   15

struct VL6180X_data {
	uint8_t i2c_address;
	uint8_t id;
	uint8_t idModelRevMajor;
	uint8_t idModelRevMinor;
	uint8_t idModuleRevMajor;
	uint8_t idModuleRevMinor;
	uint16_t idDate;
	uint16_t idTime;
	uint16_t range;
	uint8_t scaling;
	uint8_t ptp_offset;
	uint16_t io_timeout;
	bool did_timeout;
};

void VL6180X_writeReg(uint8_t address, uint16_t reg, uint8_t value);
void VL6180X_writeReg16Bit(uint8_t address, uint16_t reg, uint16_t value);
void VL6180X_writeReg32Bit(uint8_t address, uint16_t reg, uint32_t value);
uint8_t VL6180X_readReg(uint8_t address, uint16_t reg);
uint16_t VL6180X_readReg16Bit(uint8_t address, uint16_t reg);
uint32_t VL6180X_readReg32Bit(uint8_t address, uint16_t reg);

void VL6180X_setAddress(struct VL6180X_data* dev, uint8_t new_addr);
void VL6180X_init(struct VL6180X_data* dev);
void VL6180X_configureDefault(struct VL6180X_data* dev);
void VL6180X_setScaling(struct VL6180X_data* dev, uint8_t new_scaling, uint16_t timeout);
uint8_t getScaling(struct VL6180X_data* dev);
uint8_t VL6180X_readRangeSingle(struct VL6180X_data* dev);
uint8_t VL6180X_readRangeContinuous(struct VL6180X_data* dev);
uint16_t readRangeSingleMillimeters(struct VL6180X_data* dev);
uint16_t readAmbientSingle(struct VL6180X_data* dev);
uint16_t readRangeContinuousMillimeters(struct VL6180X_data* dev);
bool VL6180X_timeoutOccurred(struct VL6180X_data* dev);
uint16_t getTimeout(struct VL6180X_data* dev);
uint16_t VL6180X_readAmbientContinuous(struct VL6180X_data* dev);
void VL6180X_startRangeContinuous(struct VL6180X_data* dev, uint16_t period);
void VL6180X_startAmbientContinuous(struct VL6180X_data* dev, uint16_t period);
void VL6180X_startInterleavedContinuous(struct VL6180X_data* dev, uint16_t period);
void VL6180X_stopContinuous(struct VL6180X_data* dev);
uint8_t VL6180X_getrangestatus(struct VL6180X_data* dev);
uint8_t VL6180X_getinterruptstatus(struct VL6180X_data* dev);
void VL6180X_setrangewindow(struct VL6180X_data* dev, uint8_t max, uint8_t min);
