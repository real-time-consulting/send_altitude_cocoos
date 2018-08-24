//  Defines functions specific to the STM32 platform.
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <logger.h>
#include <Wire.h>

I2CInterface Wire;  //  Used by BME280 library.

//  Functions specific to the platform, e.g. STM32.  Called by main.cpp.
extern "C" void enable_debug(void);  //  Enable ARM Semihosting for displaying debug messages.
extern "C" void disable_debug(void); //  Disable ARM Semihosting for displaying debug messages.
extern "C" void platform_setup(void);  //  Initialise the STM32 platform.
extern "C" int test_main(void);  //  WARNING: test_main() never returns.
extern "C" void led_on(void);
extern "C" void led_off(void);
extern "C" void led_toggle(void);
static void led_setup(void);
static void led_wait(void);

//  Debugging is off by default.  Developer must switch it on with enable_debug().
static bool debugEnabled = false;

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
    //  TODO: Do some STM32 testing for now. Will be removed.
    //  test_main();  //  WARNING: test_main() never returns.
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

//////////////////////////////////////////////////////////////////////////
//  Blink code from https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void led_setup(void) {
	//  Set up Blue Pill LED GPIO.
	//  Enable GPIOC clock.
	rcc_periph_clock_enable(RCC_GPIOC);
	//  Set GPIO13 (in GPIO port C) to 'output push-pull'.
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

void led_on(void) {
	//  Switch Blue Pill LED on.
	gpio_clear(GPIOC, GPIO13);
}

void led_off(void) {
	//  Switch Blue Pill LED off.
	gpio_set(GPIOC, GPIO13);
}

void led_toggle(void) {
	//  Toggle Blue Pill LED.
	gpio_toggle(GPIOC, GPIO13);
}

static void led_wait(void) {
	for (int i = 0; i < 1500000; i++)	/* Wait a bit. */
		__asm__("nop");
}

//////////////////////////////////////////////////////////////////////////
//  STM32 Blue Pill Testing 

//  ARM Semihosting code from 
//  http://www.keil.com/support/man/docs/ARMCC/armcc_pge1358787046598.htm
//  http://www.keil.com/support/man/docs/ARMCC/armcc_pge1358787048379.htm
//  http://www.keil.com/support/man/docs/ARMCC/armcc_chr1359125001592.htm
//  https://wiki.dlang.org/Minimal_semihosted_ARM_Cortex-M_%22Hello_World%22

static int __semihost(int command, void* message) {
	//  Send an ARM Semihosting command to the debugger, e.g. to print a message.
	//  To see the message you need to run opencd and gdb concurrently:
	//    openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg
	//    arm-none-eabi-gdb -x loader.gdb
	//  loader.gdb is located at the root of this project and contains the list of gdb commands.

	//  Warning: This code will trigger a breakpoint and hang unless a debugger is connected.
	//  That's how ARM Semihosting sends a command to the debugger to print a message.
	//  This code MUST be disabled on production devices.
    __asm( 
      "mov r0, %[cmd] \n"
      "mov r1, %[msg] \n" 
      "bkpt #0xAB \n"
	:  //  Output operand list: (nothing)
	:  //  Input operand list:
		[cmd] "r" (command), 
		[msg] "r" (message)
	:  //  Clobbered register list:
		"r0", "r1", "memory"
	);
	return 0;  //  TODO
}

//  ARM Semihosting code from https://github.com/ARMmbed/mbed-os/blob/master/platform/mbed_semihost_api.c

//  ARM Semihosting Commands
#define SYS_OPEN   (0x1)
#define SYS_CLOSE  (0x2)
#define SYS_WRITE  (0x5)
#define SYS_READ   (0x6)
#define SYS_ISTTY  (0x9)
#define SYS_SEEK   (0xa)
#define SYS_ENSURE (0xb)
#define SYS_FLEN   (0xc)
#define SYS_REMOVE (0xe)
#define SYS_RENAME (0xf)
#define SYS_EXIT   (0x18)

static int semihost_write(uint32_t fh, const unsigned char *buffer, unsigned int length)
{
    if (length == 0) { return 0; }
    uint32_t args[3];
    args[0] = (uint32_t)fh;
    args[1] = (uint32_t)buffer;
    args[2] = (uint32_t)length;
    return __semihost(SYS_WRITE, args);
}

#include <string.h>

int test_main(void) {
#define SEMIHOSTING
#ifdef SEMIHOSTING
	//  Show a "hello" message on the debug console.  
	//  "semihost_write(2, ...)" means write the message to the debugger's stderr output.
	const char *msg = "hello\n";
	semihost_write(2, (const unsigned char *) msg, strlen(msg));
#endif  //  SEMIHOSTING

	//  We blink the Blue Pill onboard LED in a special pattern to distinguish ourselves
	//  from other blink clones - 2 x on, then 1 x off.
	led_setup();
	for (;;) {
		led_on(); led_wait();
		led_on(); led_wait();
		led_off(); led_wait();
	}
	return 0;
}
