// Include FreeRTOS headers.
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "lib_ee152.h"

bool do_blink_red=1, do_blink_grn=1;

#define BLINK_RED_DELAY ( 500 / portTICK_PERIOD_MS )
// Keep blinking as long as do_blink_red==true.
void task_blink_red( void * pvParameters )
{
    // Hook up the red LED to Nano D12.
    pinMode(D12, "OUTPUT");

    //write this code
    volatile bool red_led_status = 0;
    while (true) {
        if (do_blink_red) {

            digitalWrite(D12, red_led_status);
            red_led_status = !red_led_status;
            
        } else {
            digitalWrite(D12, 0);
            red_led_status = 0;
        }
        vTaskDelay(250);
    }
}

// Keep blinking as long as do_blink_grn==true.
#define BLINK_GRN_DELAY ( 500 / portTICK_PERIOD_MS )
void task_blink_grn( void * pvParameters )
{
    // The green LED is at Nano D13, or PB3.
    pinMode(D13, "OUTPUT");

    volatile bool green_led_status = 0;
    while (true) {
        if (do_blink_grn) {

            digitalWrite(D13, green_led_status);
            green_led_status = !green_led_status;
            
        } else {
            digitalWrite(D13, 0);
            green_led_status = 0;
        }
        vTaskDelay(167);
    }
}

// This task keeps reading the UART forever. It sets the globals
// do_blink_grn and do_blink_red to communicate with the other tasks.
void task_uart (void *pvParameters) {
    const char prompt[] = "R=red, G=green, B=both, N=neither: ";
    char rxByte, buf[40];
    while (1) {
        serial_write(USART2, prompt);
        rxByte = serial_read (USART2);
        int red  = (rxByte == 'R' || rxByte == 'r');
        int grn  = (rxByte == 'G' || rxByte == 'g');
        int both = (rxByte == 'B' || rxByte == 'b');
        do_blink_red = red || both;
        do_blink_grn = grn || both;
        strcpy (buf, "Red=");   strcat (buf, (do_blink_red?"on":"off"));
        strcat (buf, ", grn=");	strcat (buf, (do_blink_grn?"on\n\r":"off\n\r"));
        serial_write(USART2, buf);
    }
}

int main()
{
    // The default clock is 4MHz, which is more than fast enough for LEDs.
    //clock_setup_16MHz();		// 16 MHz
    clock_setup_80MHz();		// 80 MHz
    serial_begin (USART2);

    // Create tasks.
    TaskHandle_t task_handle_uart = NULL;
    BaseType_t OK_UART = xTaskCreate (
	    task_uart,
	    "Decide which LEDs to blink",
	    100, // stack size in words
	    NULL, // parameter passed into task, e.g. "(void *) 1"
	    tskIDLE_PRIORITY, // priority
	    &task_handle_uart);
    if (OK_UART != pdPASS) for ( ;; );

    TaskHandle_t task_handle_red = NULL;
    BaseType_t OK_red= xTaskCreate (
        task_blink_red,	// name of the task function
        "Blink Red LED",	// for debugging
        100,			// stack size in words
        NULL,		// parameter passed into task
        tskIDLE_PRIORITY + 1, // priority
        &task_handle_red);
    if (OK_red != pdPASS) for ( ;; );

    TaskHandle_t task_handle_grn = NULL;
    BaseType_t OK_green= xTaskCreate (
        task_blink_grn,	// name of the task function
        "Blink Green LED",	// for debugging
        100,			// stack size in words
        NULL,		// parameter passed into task
        tskIDLE_PRIORITY + 1, // priority
        &task_handle_grn);
    if (OK_green != pdPASS) for ( ;; );

    vTaskStartScheduler();
}