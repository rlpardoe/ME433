#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define ADDR _u(0x20) // & with last bit for read vs write -- handled by sdk I think

// hardware registers
#define IODIR _u(0x00)
#define IOCON _u(0x05)
#define GPIO _u(0x09)
#define OLAT _u(0x0A)


void set_pins(unsigned char address, unsigned char value){
    uint8_t buf[2];
    buf[0] = OLAT; // which SFR
    buf[1] = value;
    i2c_write_blocking(i2c_default, address, buf, 2, false);

}
unsigned char read_pins(unsigned char address, uint8_t *destination){
    uint8_t buf = GPIO;
    i2c_write_blocking(i2c_default, address, &buf, 1, true);
    i2c_read_blocking(i2c_default,address,destination,1,false);
}

void chip_init() {
    uint8_t buf1[2];
    buf1[0] = IODIR;
    buf1[1] = 0b0111111; // set GP7 to output, all the rest as input
    i2c_write_blocking(i2c_default, ADDR, buf1, 2, false);
    //outputs low on reset, set low anyway in case not rest since prev use
    uint8_t buf2[2];
    buf2[0] = OLAT;
    buf2[1] = 0x00;
    i2c_write_blocking(i2c_default, ADDR, buf1, 2, false);
}



int main() {

    stdio_init_all();
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    
    i2c_init(i2c_default, 100 * 1000); // 100 khz is fine
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); // using default pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    /* Just breadboard some pull ups
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    */

    chip_init(); // set inputs v outputs and starting vals

    uint8_t pin_status;

    while (1) {
        //heartbeat
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);

        // read pins and update LED output
        read_pins(ADDR, &pin_status) ;
        if ((pin_status & 0x01) == 0){ // if gp0 is low, ie button is pressed, providing path to ground
            set_pins(ADDR, 0x80); // set GP7 to high 0x80 = 1000 0000
        }
        else{
            set_pins(ADDR, 0x00);//set GP7 back to low
        }
    }
}
