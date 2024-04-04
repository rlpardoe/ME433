#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define Y_LED_PIN 14 //The GP# not the Hardware pin #, The Yellow LED is on GP14 which is hardwar pin #19
#define BUTTON_PIN 15
#define ADC_PIN 26 //ADC0/GP26, Hardware pin 31
#define VOLTS_PER_COUNT (3.3/4096)

void read_num_samples();
float convert_adc(uint16_t reading);

float voltages[100];
uint samps = 0;
uint valid = 0;
uint16_t result = 0;


int main() {
    //const uint Y_LED_PIN = 14; //The GP# not the Hardware pin #, The Yellow LED is on GP14 which is hardwar pin #19
    //const uint BUTTON_PIN = 15;
    //const uint ADC_PIN = 26; //ADC0/GP26, Hardware pin 31
    stdio_init_all();
    gpio_init(Y_LED_PIN);
    gpio_init(BUTTON_PIN);
    
    gpio_set_dir(Y_LED_PIN, GPIO_OUT);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");
    gpio_put(Y_LED_PIN, 1); // Turn YLED on

    printf("Standby, Yellow LED on, Press Button Exit Standby and turn off LED!\n");
    while(gpio_get(BUTTON_PIN) == 1){;}

    gpio_put(Y_LED_PIN,0);

    adc_init(); // init the adc module
    adc_gpio_init(ADC_PIN); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0
    
    while (1) {
        read_num_samples();
        if (valid){
            int count = 0;
            //Sampling
            while (count <samps){
                result = adc_read();
                //printf("%d",result);
                voltages[count] = convert_adc(result);
                sleep_ms(10);
                count += 1;
            }
            // Printing
            count = 0;
            while (count<samps){
                printf("Reading %d: %f\r\n", count, voltages[count]);
                count +=1;
            }
        }
        else{
            printf("Invalid Input, Try again\n");
        }
        valid = 0;
        //printf("message: %s\r\n",message);
        //sleep_ms(50);
    }
}

void read_num_samples(){
    char message[100];
    printf("Enter number of Samples to take between 0 and 100:\n");
    scanf("%s", message);
    sscanf(message,"%d", &samps);
    if ((samps >0) && (samps<=100)){
        valid = 1;}
    else{
        valid = 0;}

}

float convert_adc(uint16_t reading){
    return ((reading)*((float) VOLTS_PER_COUNT));
}
