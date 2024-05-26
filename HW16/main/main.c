#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pwm.h"

#define IN1 15 // gp15 for pwm
#define IN2 14 // gp14 for io
#define wrap_c 3000
#define div_c 1


#define LEDPin 25 // the built in LED on the Pico
#define stall 0.75 // stall pwm percentage

int put_in2 = 0;

float get_multiplier(signed int percent){
    float frac;
    float multiplier;
    //75% minimum to pass stall current/torque
    if (percent>0){
        frac = ((float)percent)/((float)100);
        //gpio_put(IN2, 0);
        put_in2 = 0;
        multiplier = frac*(1.0-stall)+stall;
        }
    else if (percent<0){
        frac = ((float)(100+percent))/((float)100);
        //gpio_put(IN2, 1);
        put_in2 = 1;
        multiplier = frac*(1.0-stall);
    }
    else if (percent==0){
        put_in2 = 0;
        return 0.0;
    }
    return multiplier;
}

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    
    printf("Start!\n");

    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);

    gpio_set_function(IN1, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(IN1); // Get PWM slice number
    float div = div_c; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = wrap_c; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    
    gpio_put(IN2, 0);
    pwm_set_gpio_level(IN1, wrap*.8);
    signed int percent;
    float multiplier;
    while (1){
        printf("Enter PWM Percentage: "); 
        scanf("%d", &percent);
        multiplier = get_multiplier(percent);
        gpio_put(IN2, put_in2);
        pwm_set_gpio_level(IN1, wrap*multiplier);
        printf("\r\nsetting to %d percent pwm or %d percent speed\r\n", ((int)(multiplier*100)), percent);

    }
}