#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <boards/pico_w.h>
#include <hardware/pio.h>
#include <hardware/gpio.h>
#include <hardware/clocks.h>

#include "urgb.h"
#include "hsv.h"
#include "ws2812b.h"
#include "blink.h"
#include "light_state.h"
#include "wifi_credentials.h"
#include "generated/ws2812.pio.h"

#include "lwipopts.h"
#include "lwip/apps/httpd.h"
#include "ssi.h"
#include "cgi.h"

#define WS2812_PIN 2
#define LIGHT_TOGGLE_PIN 15
#define STOP_BUTTON_PIN 16
#define IS_RGBW false

#define SPEED_FACTOR 1
#define DENSITY_FACTOR 1

volatile bool stop_flag = false;

void connect_to_wifi()
{
    printf("Connecting to Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000) != 0)
        printf("Attempting to connect...\n");
    printf("Connected to Wi-Fi\n");
}

void light_toggle_irq_handler(uint gpio, uint32_t events)
{
    toggle_light_state(NUM_PIXELS);
    gpio_set_irq_enabled(LIGHT_TOGGLE_PIN, GPIO_IRQ_EDGE_FALL, false);
}

void stop_button_irq_handler(uint gpio, uint32_t events)
{
    stop_flag = true;
}

enum init_result_t
{
    INIT_SUCCESS,
    STDIO_INIT_FAILURE,
    WIFI_INIT_FAILURE
};

enum init_result_t init()
{
    onboard_led_blink(250, 50);
    if (!stdio_init_all())
    {
        printf("Stdio init failed");
        return STDIO_INIT_FAILURE;
    }
    gpio_init(LIGHT_TOGGLE_PIN);
    gpio_set_dir(LIGHT_TOGGLE_PIN, GPIO_IN);
    gpio_pull_up(LIGHT_TOGGLE_PIN);
    gpio_set_irq_enabled_with_callback(LIGHT_TOGGLE_PIN, GPIO_IRQ_EDGE_FALL, false, &light_toggle_irq_handler);

    gpio_init(STOP_BUTTON_PIN);
    gpio_set_dir(STOP_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(STOP_BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(STOP_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, false, &stop_button_irq_handler);

    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed");
        return WIFI_INIT_FAILURE;
    }

    cyw43_arch_enable_sta_mode();
    connect_to_wifi();
    httpd_init();
    ssi_init(); 
    cgi_init();
    
    onboard_led_blink(250, 50);
    onboard_led_blink(250, 50);
    return INIT_SUCCESS;
}

void apply_rainbow_effect(uint* base_hue)
{
    for (uint16_t i = 0; i < NUM_PIXELS; ++i)
    {
        uint16_t hue = (*base_hue + i * 360 / NUM_PIXELS * DENSITY_FACTOR) % 360;
        uint32_t color = hue_to_rgb(hue);
        put_pixel(color);
        hue++;
    }
    *base_hue += SPEED_FACTOR;
    if (*base_hue >= 360)
    {
        printf("Resetting base hue\n");
        *base_hue -= 360;
    }
    else if (*base_hue < 0)
    {   
        printf("Resetting base hue\n");
        *base_hue += 360;
    }
}

int main()
{
    enum init_result_t init_result = init();
    if (init_result != INIT_SUCCESS)
    {
        return init_result;
    }

    printf("WS2812 LED Control, using pin %d\n", WS2812_PIN);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    set_all_blue(NUM_PIXELS, 255);

    puts("Beginning main loop");
    // uint* base_hue = (uint*)malloc(sizeof(uint));
    while (true)
    {
        if (stop_flag)
        {
            break;
        }
        if (light_state)
        {
            // apply_rainbow_effect(base_hue);
            set_all_blue(NUM_PIXELS, 255);
        }
        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP)
        {
            turn_off_all(NUM_PIXELS);
            connect_to_wifi();
        }
        sleep_ms(100);
    }
    // free(base_hue);
    cyw43_arch_deinit();
}
