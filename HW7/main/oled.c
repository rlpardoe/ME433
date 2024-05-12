#include <stdio.h>
#include "oled.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "font.h"

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

void start_i2c(){
    i2c_init(i2c_default, 100 * 1000); // 100 khz is fine
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); // using default pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
}