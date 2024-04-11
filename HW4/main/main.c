#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"


#define Y_LED_PIN 14 //The GP# not the Hardware pin #, The Yellow LED is on GP14 which is hardwar pin #19
#define V_REF 3.3
#define RESOLUTION 100

int outputval;
float wave[RESOLUTION];


int volt_to_bits(float v);
void gen_wave(uint type);
void sendperiod(float hz);


//#ifdef PICO_DEFAULT_SPI_CSN_PIN
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}
//#endif

//#if defined(spi_default) && defined(PICO_DEFAULT_SPI_CSN_PIN)
static void write_dac(uint which, uint data) {

    uint8_t buf[2];
    if (which == 0){ // DAC A
        buf[0] = (data>>6) | 0b00110000;  // write to a,no buffer, no gain, dont shutdown, first 4 bits of data
    }
    else if(which ==1){ // DAC B
        buf[0] = (data>>6) | 0b10110000;  // write to b,no buffer, no gain, dont shutdown, first 4 bits of data
    }
    buf[1] = (data<<2) & 0b000011111111 ; // get rid of first 4 bits of data; last two bits arent read

    //printf("Shifted Data: %d, %d\n",data>>6,data<<2);
    //printf("Sending: %d,%d\n", buf[0],buf[1]);

    cs_select();
    spi_write_blocking(spi_default, buf, 2);
    cs_deselect();
    sleep_ms(10);
}

/* static void read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.
    reg |= READ_BIT;
    cs_select();
    spi_write_blocking(spi_default, &reg, 1);
    sleep_ms(10);
    spi_read_blocking(spi_default, 0, buf, len);
    cs_deselect();
    sleep_ms(10);
}
*/
int main(){
    //const uint Y_LED_PIN = 14; //The GP# not the Hardware pin #, The Yellow LED is on GP14 which is hardwar pin #19
    //const uint BUTTON_PIN = 15;
    //const uint ADC_PIN = 26; //ADC0/GP26, Hardware pin 31
    stdio_init_all();
    gpio_init(Y_LED_PIN);
    gpio_set_dir(Y_LED_PIN, GPIO_OUT);

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Hello,\n");

    // This example will use SPI0 at 0.5MHz.
    //spi_init(spi_default, 500 * 1000);

    //spi_init(spi_default, 12);// Currently 12 khz eventually do 1MHZ for a smooth sin wave
    spi_init(spi_default, 10*1000*1000);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    //bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    // Make the CS pin available to picotool
    //bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

    // See if SPI is working - interrograte the device for its I2C ID number, should be 0x60
    //uint8_t id;
    //read_registers(0xD0, &id, 1);
    //printf("Chip ID is 0x%x\n", id);

    gpio_put(Y_LED_PIN, 1); // Turn YLED on




    uint i;
    while (1){
        /*outputval = volt_to_bits(1.0);
        write_dac(0,outputval);
        printf("Sending over SPI: %d,\n",outputval);
        sleep_ms(1000);
        outputval = volt_to_bits(3.3);
        write_dac(0,outputval);
        printf("Sending over SPI: %d,\n",outputval);
        sleep_ms(1000);*/
        gen_wave(1);
        i=0;
        while (i<10){
            /*sendperiod(10);
            i+=1;*/
            int j=0;
            while (j<RESOLUTION){
                outputval = volt_to_bits(wave[j]);
                write_dac(0,outputval);
                j+=1;
                sleep_ms(5);
            }
            i+=1;
        }
        gen_wave(0);
        i=0;
        while (i<5){
            /*sendperiod(1);
            i+=1;*/
            int j=0;
            while (j<RESOLUTION){
                outputval = volt_to_bits(wave[j]);
                write_dac(0,outputval);
                j+=1;
                sleep_ms(10);
            }
            i+=1;

        }

        
    }
     gpio_put(Y_LED_PIN,0);
}


void sendperiod(float hz){
    int i=0;
    while (i<RESOLUTION){
        outputval = volt_to_bits(wave[i]);
        write_dac(0,outputval);
        i+=1;
        sleep_ms((1000.0/hz)/((float)RESOLUTION));
    }
    
}

int volt_to_bits(float v){
    if (v > V_REF){
        return 0;
    }
    uint out ;
    out = (int) (((v)/V_REF)*(1024)); // 1024 bc 10 bit dac
    return out;
}

void gen_wave(uint type){
    // 0 means triangle
    // 1 means sin
    int i = 0;
    if (type == 0){
        while (i<RESOLUTION){
            if (i<(RESOLUTION/2)){
                wave[i] = V_REF*(((float) i)/(RESOLUTION/2));
            }
            else{
                wave[i] = V_REF*(((float) (RESOLUTION-i))/(RESOLUTION/2));
            }
            i+=1;
        }
    }
    else if (type ==1){
        while (i< RESOLUTION){
            wave[i] =((V_REF/2)+ (V_REF/2) * sin(2* M_PI * (((float) i)/(RESOLUTION-1))));
            i+=1;
        } 
    }
       

}