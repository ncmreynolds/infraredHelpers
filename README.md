# Infrared helpers
This library consists of helper classes to send and receive signals over IR, one class for transmit, one for receive, per architecture.

These helpers are for when you want to make your own custom encoding/decoding and do so just by adding symbols to a buffer then calling a send function or parsing a buffer of timings yourself. Using these helper functions you can use the same code (with some minor limitations) across different platforms.

It is a solution in search of the author's own very specific Laser-Tag related problem.

## Target architectures

I would like this to be able to 'genericise' infrared transmission/reception across different hardware to make building things simpler. There are IR remote control libraries already for this but they are very focused on TV style remote controls.

### ESP32

These helpers use the ESP32 RMT peripheral and do all the configuration/initialisation of it for you. In principle sending IR signals is the main role of RMT but most examples are either for known encodings or uses of it to drive other custom peripherals with tight timing requirement using hardware. 

### AVR

These helpers use interrupts and PWM/bit-banging. **Not yet fully implemented.**

### RP2040/2035

**Not yet implemented.**

## API

The API is still quite low-level, with the application feeding it microsecond pulse timings to send. The library's purpose is to do all the boilerplate and tiresome accurate timing work. Mostly it is intended to be a dependency in other libraries that implement infrared communication.

#### IR Transmission

The library sets up an instance of the class appropriate for the architecture called 'transmitHelper' and these are its methods...

	//Symbol count
	uint16_t getMaximumNumberOfSymbols() );				//Maximum number of symbols
	bool setMaximumNumberOfSymbols(uint16_t symbols);	//Must be done before begin(), default varies by platform
	uint8_t getMinimumNumberOfSymbols();				//Minimum number of symbols, which varies by platform
	bool setCarrierFrequency(uint16_t frequency);					//Must be done before begin(), default is 56000
	bool setDutyCycle(uint8_t duty, uint8_t transmitterIndex = 0);	//Must be done before begin(), default is 50 and very unlikely to change
	bool begin(uint8_t numberOfTransmitters = 1);		//Initialise one or more transmitters
	bool configureTxPin(uint8_t index, int8_t pin);		//Configure a pin for TX on the current available channel
	bool addSymbol(uint8_t index, uint16_t duration0, uint8_t level0, uint16_t duration1, uint8_t level1);	//Add a symbol to the buffer for the specified transmitter channel
	bool transmitSymbols(uint8_t transmitterIndex, bool wait = false);	//Transmit a buffer from the specified transmitter channel
	bool transmitterBusy(uint8_t index);				//Used to check if busy before starting another transmission

#### IR Transmission example

To Do

#### IR reception

To Do

#### IR reception example

To Do

## Use in other libraries

The first public use is in my [library to handle MilesTag 2](https://github.com/ncmreynolds/milesTag) IR transmission/reception as used in the LaserWar Laser-Tag system.
