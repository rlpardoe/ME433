/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/binary_info.h"
#include "ssd1306.h"
#include "oled.h"


/// \tag::uart_advanced[]

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;
char checked_char;
char message[100];

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        //sscanf(&checked_char, "%c", ch);
        //uart_putc(UART_ID, ch);
        if ((ch == 0)|(ch == 13)){ // null char or carrige return
            //write to screen
            //uart_putc(UART_ID, 's');
            ssd1306_clear();
            draw_message(message,0,0);
            ssd1306_update();
            uart_puts(UART_ID, message);
            uart_putc(UART_ID, '\r');
            uart_putc(UART_ID, '\n');

            for (int i = 0; i<=chars_rxed; i++){ // clear old message
                message[i] = '\0'; //if next message is shorter, when overwritten will leave \0 at end of message
            }
            chars_rxed = 0;
        }
        else{
            message[chars_rxed] = ch;
            chars_rxed++;
        }
        //my psuedo code
        //store char
        //if char is /n store and draw message
        // if you dont scanf when you see /n store null charachter literally char 0
        //possibly sscanf to check that it is a letter and coms working properly

        // Echoing
        // Can we send it back?
        //if (uart_is_writable(UART_ID)) {
            // Change it slightly first!
            //uart_putc(UART_ID, ch);
        //}
    }
}

int main() {
    // Set up our UART with a basic baud rate.



    // init LED
    stdio_init_all();
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    

    //Init UART i2c and oled
    uart_init(UART_ID, 115200);
    start_i2c();
    ssd1306_setup(); // Problem


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

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHello, uart interrupts\r\n");

    while (1)
        gpio_put(LED_PIN, 1);
        sleep_ms(200);
        gpio_put(LED_PIN, 0);
}

/// \end:uart_advanced[]
