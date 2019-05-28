#include <application.h>
#include "bc_sc16is740.h"

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Write and read FIFOs for RS-485 Module async tranfers
bc_fifo_t write_fifo;
bc_fifo_t read_fifo;
uint8_t write_fifo_buffer[512];
uint8_t read_fifo_buffer[512];

void send_data();

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
        send_data();
    }
}

void module_rs485_event_handler(bc_module_rs485_event_t event, void *param)
{
    (void) param;

    //bc_log_debug("Event %d", event);

    if (event == BC_MODULE_RS485_EVENT_VOLTAGE)
    {
        float voltage;
        bc_module_rs485_get_voltage(&voltage);
        bc_log_debug("Voltage %f", voltage);
    }

    if (event == BC_MODULE_RS485_EVENT_ASYNC_WRITE_DONE)
    {
        bc_log_debug("Async write DONE");
    }

    if (event == BC_MODULE_RS485_EVENT_ASYNC_READ_DATA)
    {
        bc_log_debug("Async read DATA");
        static uint8_t rx_buffer[32];
        size_t b = bc_module_rs485_async_read(rx_buffer, sizeof(rx_buffer));

        bc_log_dump(rx_buffer, b, "RX bytes %d", b);
    }
}

void application_init(void)
{
    bc_system_deep_sleep_disable();

    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Init FIFOs
    bc_fifo_init(&write_fifo, write_fifo_buffer, sizeof(write_fifo_buffer));
    bc_fifo_init(&read_fifo, read_fifo_buffer, sizeof(read_fifo_buffer));

    // Init RS-485 Module
    bc_module_rs485_init();
    bc_module_rs485_set_event_handler(module_rs485_event_handler, NULL);
    bc_module_rs485_set_update_interval(10000);
    bc_module_rs485_set_baudrate(BC_MODULE_RS485_BAUDRATE_9600);
    bc_module_rs485_set_async_fifo(&write_fifo, &read_fifo);

    // Test async write
    bc_module_rs485_async_write((uint8_t*)0x00, 100);

    // Start async reading
    bc_module_rs485_async_read_start(10);

}

void send_data()
{
    //static uint8_t buffer[] = "ABC";
    //bc_module_rs485_write(buffer, sizeof(buffer));

    // Send radnom data from address 0x00 from FLASH memory
    bc_module_rs485_async_write((uint8_t*)0x00, 100);
}
/*
void application_task(void)
{
    size_t data_size;
    static uint8_t rx_buffer[32];

    bc_module_rs485_available(&data_size);

    bc_log_debug("Available RX: %d", data_size);

    if (data_size)
    {
        size_t b = bc_module_rs485_read(rx_buffer, sizeof(rx_buffer), 0);

        bc_log_dump(rx_buffer, b, "RX bytes %d", b);
    }

    // Plan next run this function after 1000 ms
    bc_scheduler_plan_current_from_now(500);
}
*/
