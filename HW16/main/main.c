#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pwm.h"

#define IN1 15 // gp15 for pwm
#define IN2 14 // gp14 for io
#define IN3 13 // gp13 for pwm
#define IN4 12 // gp12 for io
#define wrap_c 3000
#define div_c 1
#define LBOUND 40
#define RBOUND 60

#define LEDPin 25 // the built in LED on the Pico
#define stall 0.75 // stall pwm percentage

int put_in2 = 0;
int put_in4 = 0;


float get_multiplier(float percent, int* io){
    float frac;
    float multiplier;
    //75% minimum to pass stall current/torque
    if (percent>0){
        frac = ((float)percent)/((float)100);
        //gpio_put(IN2, 0);
        *io = 0;
        multiplier = frac*(1.0-stall)+stall;
        }
    else if (percent<0){
        frac = ((float)(100+percent))/((float)100);
        //gpio_put(IN2, 1);
        *io = 1;
        multiplier = frac*(1.0-stall);
    }
    else if (percent==0){
        *io = 0;
        return 0.0;
    }
    return multiplier;
}

void setmotors(int input){ // input ranges 0 to 100
    // implement linear funct to map 1 number input to two command percentages for each 
    float L_command;
    float R_command;
    float L_multiplier;
    float R_multiplier;
    printf("\r\nInput: %d", input);
    if (input == 0){ //stop motors
        L_multiplier =0;
        R_multiplier =0;

        put_in4 = 0;
        put_in2 = 0;
    }
    else{
        if (input <= LBOUND){
            R_command = 100.0;
            L_command = ((float)(input))*(100.0/((float)LBOUND)); 
            printf("\r\nline is Left, L_command: %f",L_command);
        }
        else if (input >= RBOUND){
            L_command = 100.0;
            R_command = ((float)(100.0-input))*(100.0/((float)(100-RBOUND)));
            printf("\r\nline is Right, R_command: %f",R_command);

        }
        else if (input > 40 && input < 70){
            L_command = 100.0;
            R_command = 100.0;
        }
        
        L_multiplier = get_multiplier(L_command, &put_in4);
        R_multiplier = get_multiplier(R_command, &put_in2);
    }
    
    

    gpio_put(IN4, put_in4);
    pwm_set_gpio_level(IN3, wrap_c*L_multiplier);
    gpio_put(IN2, put_in2);
    pwm_set_gpio_level(IN1, wrap_c*R_multiplier);

    printf("\r\n L:%d percent R:%d percent\r\n", ((int)(L_multiplier*100)), ((int)(R_multiplier*100)));
}

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    
    printf("Start!\n");

    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);

    gpio_init(IN4);
    gpio_set_dir(IN4, GPIO_OUT);

    gpio_set_function(IN1, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num_r = pwm_gpio_to_slice_num(IN1); // Get PWM slice number
    float div_r = div_c; // must be between 1-255
    pwm_set_clkdiv(slice_num_r, div_r); // divider
    uint16_t wrapr = wrap_c; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num_r, wrapr);
    pwm_set_enabled(slice_num_r, true); // turn on the PWM

    gpio_set_function(IN3, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num_l = pwm_gpio_to_slice_num(IN3); // Get PWM slice number
    float div_l = div_c; // must be between 1-255
    pwm_set_clkdiv(slice_num_l, div_l); // divider
    uint16_t wrapl = wrap_c; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num_l, wrapl);
    pwm_set_enabled(slice_num_l, true); // turn on the PWM
    
    gpio_put(IN2, 0);
    pwm_set_gpio_level(IN1, wrapr*.8);

    gpio_put(IN4, 0);
    pwm_set_gpio_level(IN3, wrapl*.8);
    int uinput;
    while (1){
        printf("Enter Command: "); 
        scanf("%d", &uinput);
        printf("\r\nrecieved: %d", uinput);
        setmotors(uinput);
    }
}