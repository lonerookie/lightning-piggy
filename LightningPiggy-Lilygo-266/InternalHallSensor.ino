#include <driver/adc.h>   // for read_internal_hall_sensor

// This allows detecting magnets close to the ESP32 chip.
//
// Testing on the 2.66 inch:
//
// At 100 reads:
// The regular value without magnets to the ESP32 is between 3.5 and 5.5
// A magnet directly to the ESP32 (so underside of PCB) will bring it down to between -1.5 and 1.5
// A magnet on the other side of the ESP32 (left side of the display) will bring it between 6.0 and 8.5
//
// Doing 1000 reads brings these values down to:
// between 2.0 and 4.0
// between -0.5 and 0.5
// between 3.5 and 5.5
//
// At least 100 reads are needed to produce a somewhat stable value.
// 1000 reads take around 100ms, 100 reads around 10ms.
//
// Pushing the button (GPIO39) scales the value up by 50%, this has something to do with the hall amplifier.
// 
double read_internal_hall_sensor() {
  adc1_config_width(ADC_WIDTH_12Bit);

	double total = 0;
	for (double i=0;i<500;i++) {
		total += hall_sensor_read();
	}

	return total/500;
}
