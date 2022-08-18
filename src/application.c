#include <application.h>

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Write and read FIFOs for RS-485 Module async tranfers
twr_fifo_t write_fifo;
twr_fifo_t read_fifo;
uint8_t write_fifo_buffer[512];
uint8_t read_fifo_buffer[512];

void send_data();

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        uint8_t toggle_relay_0[] = {0x01, 0x06, 0x00, 0x01, 0x03, 0x00, 0xD8, 0xFA};
        twr_module_rs485_async_write(toggle_relay_0, sizeof(toggle_relay_0));

        twr_led_set_mode(&led, TWR_LED_MODE_TOGGLE);
        send_data();
    }
}

void module_rs485_event_handler(twr_module_rs485_event_t event, void *param)
{
    (void) param;

    if (event == TWR_MODULE_RS485_EVENT_VOLTAGE)
    {
        float voltage;
        twr_module_rs485_get_voltage(&voltage);
        twr_log_debug("%f", voltage);

        char b[20];
        snprintf(b, sizeof(b), "%f\n", voltage);
        twr_uart_write(TWR_UART_UART2, b, strlen(b));
    }

    if (event == TWR_MODULE_RS485_EVENT_ASYNC_WRITE_DONE)
    {
        twr_log_debug("Async write DONE");
    }

    if (event == TWR_MODULE_RS485_EVENT_ASYNC_READ_DATA)
    {
        twr_log_debug("Async read DATA");
        static uint8_t rx_buffer[32];
        size_t b = twr_module_rs485_async_read(rx_buffer, sizeof(rx_buffer));

        twr_log_dump(rx_buffer, b, "RX bytes %d", b);
    }

    if (event == TWR_MODULE_RS485_EVENT_ASYNC_READ_TIMEOUT)
    {
        // Async receive timeout event
    }
}

void application_init(void)
{
    twr_system_deep_sleep_disable();

    // Initialize logging
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_set_mode(&led, TWR_LED_MODE_ON);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Init FIFOs
    twr_fifo_init(&write_fifo, write_fifo_buffer, sizeof(write_fifo_buffer));
    twr_fifo_init(&read_fifo, read_fifo_buffer, sizeof(read_fifo_buffer));

    // Init RS-485 Module
    twr_module_rs485_init();
    twr_module_rs485_set_event_handler(module_rs485_event_handler, NULL);
    twr_module_rs485_set_update_interval(5000);
    twr_module_rs485_set_baudrate(TWR_MODULE_RS485_BAUDRATE_9600);
    twr_module_rs485_set_async_fifo(&write_fifo, &read_fifo);

    // Test async write
    twr_module_rs485_async_write((uint8_t*)0x00, 100);

    // Start async reading
    twr_module_rs485_async_read_start(10);

}

void send_data()
{
    //static uint8_t buffer[] = "ABC";
    //twr_module_rs485_write(buffer, sizeof(buffer));

    // Send radnom data from address 0x00 from FLASH memory
    twr_module_rs485_async_write((uint8_t*)0x00, 100);
}
/*
void application_task(void)
{
    size_t data_size;
    static uint8_t rx_buffer[32];

    twr_module_rs485_available(&data_size);

    twr_log_debug("Available RX: %d", data_size);

    if (data_size)
    {
        size_t b = twr_module_rs485_read(rx_buffer, sizeof(rx_buffer), 0);

        twr_log_dump(rx_buffer, b, "RX bytes %d", b);
    }

    // Plan next run this function after 1000 ms
    twr_scheduler_plan_current_from_now(500);
}

*/
