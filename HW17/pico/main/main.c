#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/binary_info.h"
#include "hardware/pwm.h"

//pwm
#define IN1 15 // gp15 for pwm
#define IN2 14 // gp14 for io
#define IN3 13 // gp13 for pwm
#define IN4 12 // gp12 for io
#define wrap_c 3000
#define div_c 1
#define LBOUND 40
#define RBOUND 60
#define HRES 640
#define LEDPin 25 // the built in LED on the Pico
#define stall 0.75 // stall pwm percentage
//uart
#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

float get_multiplier(float percent, int* io);
void setmotors(int input);
void on_uart_rx();


//UART setup and helpers
static int chars_rxed = 0;
//char checked_char;
char message[100];

int uinput;

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        //sscanf(&checked_char, "%c", ch);
        //uart_putc(UART_ID, ch);
        if ((ch == 0)|(ch == 13)){ // null char or carrige return
            printf("From PiZero: %s\r\n",message);
           
            sscanf(message,"%d", &uinput);
            setmotors((int)(uinput*100/HRES));

            for (int i = 0; i<=chars_rxed; i++){ // clear old message
                message[i] = '\0'; //if next message is shorter, when overwritten will leave \0 at end of message
            }
            chars_rxed = 0;
            
        }
        else{
            message[chars_rxed] = ch;
            chars_rxed++;
        }
        
    }
}



//PWM setup and helpers
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
    //printf("\r\nInput: %d", input);
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
            //printf("\r\nline is Left, L_command: %f",L_command);
        }
        else if (input >= RBOUND){
            L_command = 100.0;
            R_command = ((float)(100.0-input))*(100.0/((float)(100-RBOUND)));
            //printf("\r\nline is Right, R_command: %f",R_command);

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

    //printf("\r\n L:%d percent R:%d percent\r\n", ((int)(L_multiplier*100)), ((int)(R_multiplier*100)));
}

int main() {
    stdio_init_all();

    /*while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");*/

    //setup UART
    //Init UART i2c and oled
    uart_init(UART_ID, 115200);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    printf("\nHello, uart interrupts\r\n");

    //init PWM
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
    pwm_set_gpio_level(IN1, wrapr*0);

    gpio_put(IN4, 0);
    pwm_set_gpio_level(IN3, wrapl*0);
    int uinput;

    while (1){
        tight_loop_contents();
    }
}