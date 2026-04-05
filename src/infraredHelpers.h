#ifndef infraredHelpers_h
#define infraredHelpers_h
#include <Arduino.h>						//Standard Arduino library


class infraredHelpers
{
	public:
		infraredHelpers();																//Constructor function
		~infraredHelpers();																//Destructor function

		//Symbol count
		uint16_t getMaximumNumberOfSymbols() __attribute__((always_inline));			//Maximum number of symbols
		bool setMaximumNumberOfSymbols(uint16_t symbols);								//Must be done before begin(), default varies by platform
		uint8_t getMinimumNumberOfSymbols();											//Minimum number of symbols, which varies by platform
		//Message length
		uint8_t getMaximumMessageLength(uint8_t length);								//Get the maximum message length
		void setMaximumMessageLength(uint8_t length);									//Set the maximum message length
		
		//Debug
		void debug(Stream &);															//Enable debugging on a stream, eg. Serial, which must already be started
	
	protected:
		uint8_t maximum_message_length_ = 2;											//Maximum size of a message, without preamble, start bits etc.
		#if CONFIG_IDF_TARGET_ESP32
			uint16_t maximum_number_of_symbols_ = 64;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32S2
			uint16_t maximum_number_of_symbols_ = 64;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32S3
			uint16_t maximum_number_of_symbols_ = 64;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32C2
			uint16_t maximum_number_of_symbols_ = 48;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32C3
			uint16_t maximum_number_of_symbols_ = 48;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32C6
			uint16_t maximum_number_of_symbols_ = 48;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32H2
			uint16_t maximum_number_of_symbols_ = 64;									//Default to minimum
		#elif CONFIG_IDF_TARGET_ESP32P4
			uint16_t maximum_number_of_symbols_ = 64;									//Default to minimum
		#else
			uint16_t maximum_number_of_symbols_ = 64;									//Default to minimum
		#endif

		Stream *debug_uart_ = nullptr;													//The stream used for debugging
	
	private:
		#if CONFIG_IDF_TARGET_ESP32
			uint8_t minimum_number_of_symbols_ = 64;									//64 is ESP32 classic minimum
		#elif CONFIG_IDF_TARGET_ESP32S2
			uint8_t minimum_number_of_symbols_ = 64;									//64 is ESP32S2 minimum
		#elif CONFIG_IDF_TARGET_ESP32S3
			uint8_t minimum_number_of_symbols_ = 64;									//64 is ESP32S3 minimum
		#elif CONFIG_IDF_TARGET_ESP32C2
			uint8_t minimum_number_of_symbols_ = 48;									//48 is ESP32C2 minimum
		#elif CONFIG_IDF_TARGET_ESP32C3
			uint8_t minimum_number_of_symbols_ = 48;									//48 is ESP32C3 minimum
		#elif CONFIG_IDF_TARGET_ESP32C6
			uint8_t minimum_number_of_symbols_ = 48;									//48 is ESP32C6 minimum
		#elif CONFIG_IDF_TARGET_ESP32H2
			uint8_t minimum_number_of_symbols_ = 64;									//64 is ESP32H2 minimum
		#elif CONFIG_IDF_TARGET_ESP32P4
			uint8_t minimum_number_of_symbols_ = 64;									//64 is ESP32P4 minimum
		#else
			uint8_t minimum_number_of_symbols_ = 64;									//64 is an assumed default ESP32 minimum
		#endif
};
inline uint16_t infraredHelpers::getMaximumNumberOfSymbols()	//Maximum number of symbols
{
	return maximum_number_of_symbols_;
}

class infraredTransmitHelper
{
	public:
		infraredTransmitHelper();														//Constructor function
		~infraredTransmitHelper();														//Destructor function
		virtual bool setCarrierFrequency(uint16_t frequency);							//Must be done before begin(), default is 56000
		virtual bool setDutyCycle(uint8_t duty, uint8_t transmitterIndex = 0);			//Must be done before begin(), default is 50 and very unlikely to change
		virtual bool begin(uint8_t numberOfTransmitters);								//Set up transmitters
		virtual bool configureTxPin(uint8_t index, int8_t pin);							//Configure a pin for TX on the specified channel
		virtual bool addSymbol(uint8_t index, uint16_t duration0, uint8_t level0,		//Add a symbol to the buffer for the specified transmitter channel
			uint16_t duration1, uint8_t level1);
		virtual bool transmitSymbols(uint8_t transmitterIndex,							//Transmit a buffer from the specified transmitter channel
				bool wait = false);
		virtual bool transmitterBusy(uint8_t index);									//Used to check if busy before starting another transmission

	protected:
		uint8_t number_of_transmitters_ = 0;											//Number of transmitter channels, usually 1-2
		uint8_t* number_of_symbols_to_transmit_ = nullptr;								//Count of symbols to avoid going past end of buffer
	
	private:
};

class infraredReceiveHelper
{
	public:
		infraredReceiveHelper();														//Constructor function
		~infraredReceiveHelper();														//Destructor function
		virtual bool begin(uint8_t numberOfReceivers);									//Set up receivers
		virtual bool configureRxPin(uint8_t index, int8_t pin, bool inverted = true);	//Configure a pin for RX on the current available channel
		virtual uint8_t receivedSymbolLevel0(uint8_t index, uint16_t symbolIndex);		//Getter for the symbol data
		virtual uint8_t receivedSymbolLevel1(uint8_t index, uint16_t symbolIndex);		//Getter for the symbol data
		virtual uint16_t receivedSymbolDuration0(uint8_t index, uint16_t symbolIndex);	//Getter for the symbol data
		virtual uint16_t receivedSymbolDuration1(uint8_t index, uint16_t symbolIndex);	//Getter for the symbol data
		virtual uint8_t getNumberOfReceivedSymbols(uint8_t index);						//Number of symbols received by a receiver
		virtual void resume(uint8_t index);												//Resume reception on a specific channel

	protected:
		uint8_t number_of_receivers_ = 0;												//Number of receiver channels, usually 1
		uint8_t* number_of_received_symbols_ = nullptr;									//Count of symbols in the buffer
	
	private:
};

#if defined ESP32
	#include <esp32rmtHelpers.h>			//Use the RMT peripheral for ESP32
#elif defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_NANO
	#include <avrInfraredHelpers.h>			//Use bitbang and interrupts
#endif

#endif