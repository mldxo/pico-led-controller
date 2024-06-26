#include "blink.h"

#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>

#include "light_state.h"

void onboard_led_blink(uint32_t on_time, uint32_t off_time)
{
    if (on_time < 10)
        on_time = 10;
    if (off_time < 10)
        off_time = 10;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(on_time);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(off_time);
}

void led_blink(uint32_t on_time, uint32_t off_time)
{
    if (on_time < 10)
        on_time = 10;
    if (off_time < 10)
        off_time = 10;
    toggle_light_state();
    sleep_ms(on_time);
    toggle_light_state();
    sleep_ms(off_time);
}
