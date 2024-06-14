#ifndef _SSI_H
#define _SSI_H

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#include "light_state.h"

const char* ssi_tags[] = {"volt","temp","onboard_led","led"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t print_value;
  switch (iIndex)
  {
    case 0: // volt
        {
            const float voltage = adc_read() * 3.3f / (1 << 12);
            print_value = snprintf(pcInsert, iInsertLen, "%f", voltage);
        }
        break;
    case 1: // temp
        {
            const float voltage = adc_read() * 3.3f / (1 << 12);
            const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
            print_value = snprintf(pcInsert, iInsertLen, "%f", tempC);
        }
        break;
    case 2: // onboard led
        {
            bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
            if (led_status)
                print_value = snprintf(pcInsert, iInsertLen, "ON");
            else
                print_value = snprintf(pcInsert, iInsertLen, "OFF");
        }
        break;
    case 3: // ws2812b led
        {
            if (light_state)
                print_value = snprintf(pcInsert, iInsertLen, "ON");
            else
                print_value = snprintf(pcInsert, iInsertLen, "OFF");
        }
        break;
    default:
        print_value = 0;
        break;
  }
  return (u16_t)print_value;
}

void ssi_init()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}

#endif
