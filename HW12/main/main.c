#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pwm.h"

#define SERVO_PIN 15 // gp15 for pwm
#define PWM_RANGE 0.1075
#define PWM_MIN 0.0225
#define wrap_c 62500
#define div_c 40
#define TRAVEL_TIME 4/2
#define UPDATE_TIME 0.001

#define LEDPin 25 // the built in LED on the Pico


float angle_to_pwm(float angle);

void travel(int start_angle, int end_angle){
    int steps = ((float)TRAVEL_TIME)/((float)UPDATE_TIME);
    float angle_step = ((float)(end_angle-start_angle))/((float)steps);
    float pwm_factor;
    for (int i =0; i<=steps; i+= 1 ){
        pwm_factor = angle_to_pwm(start_angle+(angle_step*i));
        pwm_set_gpio_level(SERVO_PIN, wrap_c*pwm_factor); // set the duty cycle to 50%
        sleep_ms(UPDATE_TIME*1000);
    }

}

float angle_to_pwm(float angle){
    //scale
    float factor = (angle/180)*PWM_RANGE; //for 50hz 5% of pwm travel is 1ms 10% is 2ms 
    //ofsset
    factor = factor + PWM_MIN;
    return factor;
}

int main() {
    stdio_init_all();
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN); // Get PWM slice number
    float div = div_c; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = wrap_c; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    //pwm_set_gpio_level(SERVO_PIN, wrap*0.1); // set the duty cycle to 50%
    pwm_set_gpio_level(SERVO_PIN, wrap*angle_to_pwm(0));
    sleep_ms(2000);
    while (1){
        travel(0,180);
        travel(180,0);
    }
}