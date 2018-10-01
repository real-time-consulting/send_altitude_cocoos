//  Defines functions specific to the STM32 Blue Pill platform.
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <logger.h>
#include "bluepill.h"

//  Debugging is off by default.  Developer must switch it on with enable_debug().
static bool debugEnabled = false;

extern "C" int micropython_main(int argc, char **argv);  ////  TODO: MicroPython

void platform_setup(void) {
    //  Initialise the STM32 platform. At startup, the onboard LED will blink on-off-on-off-on and stays on.
	//  If LED blinks on-off-on-off and stays off, then debug mode is enabled and no debugger is connected.
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	led_setup();
	if (debugEnabled) {
		led_on(); led_wait();
		led_off(); led_wait();
		led_on(); led_wait();
		led_off(); led_wait();
		//  This line will call ARM Semihosting and may hang until debugger is connected.
  		debug_println("----platform_setup");
		led_on();
	}
	micropython_main(0, NULL); for(;;) {} ////  TODO: MicroPython
}

void enable_debug(void) {
	//  Enable ARM Semihosting for displaying debug messages.
	debugEnabled = true;
	enable_log();
}

void disable_debug(void) {
	//  Disable ARM Semihosting for displaying debug messages.
	debugEnabled = false;
	disable_log();
}

//  For Legacy Arduino Support only...

#define ALL_PINS_SIZE (sizeof(allPins) / sizeof(uint32_t))
static const uint32_t allPins[] = {  //  Map Arduino pin number to STM32 port ID.
	0,  //  Unknown pin.
	SPI1,
	SPI2,
	I2C1,
	I2C2,
	USART1,
	USART2,
};  //  TODO: Support STM32 alternate port mapping.

uint32_t convert_pin_to_port(uint8_t pin) {
	//  Map Arduino Pin to STM32 Port, e.g. 1 becomes SPI1
	if (pin < 1 || pin >= ALL_PINS_SIZE) { return 0; }  //  Invalid pin.
	return allPins[pin];
}

uint8_t convert_port_to_pin(uint32_t port_id) {
	//  Map STM32 port to Arduino Pin, e.g. SPI1 becomes 1
	for (uint8_t pin = 1; pin < ALL_PINS_SIZE; pin++) {
		if (port_id == allPins[pin]) { return pin; }
	}
	return 0;  //  Invalid port.
}

//  Force the linker not to link in the RCC clock functions that Blue Pill doesn't use.  Doing this will save 1KB in ROM space.
extern "C" void rcc_clock_setup_in_hse_12mhz_out_72mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
extern "C" void rcc_clock_setup_in_hse_16mhz_out_72mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
extern "C" void rcc_clock_setup_in_hse_25mhz_out_72mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
extern "C" void rcc_clock_setup_in_hse_8mhz_out_24mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
extern "C" void rcc_clock_setup_in_hsi_out_24mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
extern "C" void rcc_clock_setup_in_hsi_out_48mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
extern "C" void rcc_clock_setup_in_hsi_out_64mhz(void) { rcc_clock_setup_in_hse_8mhz_out_72mhz(); }
