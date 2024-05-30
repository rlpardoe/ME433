#include "imu_helpers.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

void writeIMU(unsigned char address, unsigned char reg, unsigned char value){
    uint8_t buf[2];
    buf[0] = reg; // which SFR
    buf[1] = value;
    i2c_write_blocking(i2c_default, address, buf, 2, false);

}
unsigned char readIMU(unsigned char address, unsigned char *destination){
    uint8_t buf = ACCEL_XOUT_H;
    i2c_write_blocking(i2c_default, address, &buf, 1, true);
    i2c_read_blocking(i2c_default,address,destination,14,false);
}

/*int char2int(char high, char low){
  int* putloc;
  *putloc = high;
  *(putloc + 1) = low;
  return *putloc;
}

void convertBytes(unsigned char* raw, int* accel, signed int* rot int* temp){
  //
}
*/
char start_IMU(){
    char I_am;
    writeIMU(IMU_ADDRESS, PWR_MGMT_1, 0x00);
    //set accell config to +- 2g
    // set AFS_SEL (bits 4,3 of ACCEL_CONGIF) = 0, all else 0, they trig self tests
    writeIMU(IMU_ADDRESS, ACCEL_CONFIG, 0x00);
    //set gyro config to +-2000 dps
     // set FS_SEL (bits 3,4 of GYRO_COFIG) =  3, all else 0, they trig self tests
    writeIMU(IMU_ADDRESS, GYRO_CONFIG, 0b00011000);

    //get whoami
    uint8_t buf = WHO_AM_I;
    i2c_write_blocking(i2c_default, IMU_ADDRESS,&buf, 1, true);
    i2c_read_blocking(i2c_default,IMU_ADDRESS,&I_am,1,false);
    return I_am;
}

void start_i2c(){
    i2c_init(i2c_default, 100 * 1000); // 100 khz is fine
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); // using default pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
}