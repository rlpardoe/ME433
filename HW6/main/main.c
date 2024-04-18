#include <stdio.h>
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "font.h"

#define ADC_PIN 26 //ADC0/GP26, Hardware pin 31
#define VOLTS_PER_COUNT (3.3/4096)


void draw_message(char *p, int x, int y);
void write_letter(unsigned char c, int x, int y);
float convert_adc(uint16_t reading);
void start_i2c();
void start_adc();


int main(){
    stdio_init_all();
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    start_i2c();
    start_adc();

    //while (!stdio_usb_connected()) {
        //sleep_ms(100);
    //}

    ssd1306_setup(); // set inputs v outputs and starting vals
    //printf("ssd1306 setup\n");

    char message[50];

    int t1,t2;
    float adc_volatage, fps;
    ssd1306_clear();

    //bootstrapping timer i e first cycle outside of loop, prints time elapsed as 0
    t1 = to_us_since_boot(get_absolute_time());
    gpio_put(LED_PIN, 1);
    adc_volatage = convert_adc(adc_read());
    fps = 0;
    sprintf(message, "ADC0: %3.2fV",adc_volatage);
    draw_message(message,5,0);
    ssd1306_update();
    gpio_put(LED_PIN, 0);

    while (1) {
        t2 = to_us_since_boot(get_absolute_time());// time returned in microseconds
        gpio_put(LED_PIN, 1); // heartbeat
        fps = ((float) 1000000)/(t2-t1); 
        printf("%f, %f",t2,t1);
        t1 = t2;
        adc_volatage = convert_adc(adc_read());
        sprintf(message, "ADC0: %3.2fV\n\n\nFPS: %4.1f",adc_volatage,fps);
        // always printing fps based on time it took to display prev frame
        draw_message(message,5,0);
        ssd1306_update();
        gpio_put(LED_PIN, 0);
    }
}

void draw_message(char *p, int x, int y){
    int i = 0, j =0;
    unsigned char printChar = *(p+i);
    unsigned int x_pos = x, y_pos = y;

    if (x<125 && y<26){ 
        while (printChar != '\0'){

            if ((x_pos>124) | (printChar == '\n')){ // if cannot fit charachter to right
                y_pos = y_pos+8; // go to next 'line'
                if (y_pos < 26){ // if there is still a next line to go to
                    x_pos = x; // reset x to initial value, equivalent of return
                    i++; // increment counter
                    printChar = *(p+i); // grab new char
                    continue;
                }
                else{ // if there isnt a next line to go to 
                    printf("message too large to display\n");
                    break;
                }
            }

            write_letter(printChar, (x_pos), y_pos);

            i++; // increment counter
            printChar = *(p+i); // grab new char
            x_pos += 5; // move 'cursor' right
        }
    }
    else{
        printf("not enough space!\n");
    }
}

void write_letter(unsigned char c, int x, int y){ // x and y are top left corner
    unsigned char temp;
    for(int i = 0; i<5; i++ ){
        temp = ASCII[c-32][i];
        printf("%c, %d, %d\n",c,(c-20),temp);
        for (int j =0; j<8; j++){
            if ((temp>>(j)) & 0b1){
                ssd1306_drawPixel(x+i,y+j,1);
            }
            else{
                ssd1306_drawPixel(x+i,y+j,0);
            }
        }

    }
}

float convert_adc(uint16_t reading){
    return ((reading)*((float) VOLTS_PER_COUNT));
}

void start_i2c(){
    i2c_init(i2c_default, 100 * 1000); // 100 khz is fine
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); // using default pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
}

void start_adc(){
    adc_init(); // init the adc module
    adc_gpio_init(ADC_PIN); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0
}
