#ifndef infraredHelpers_cpp
#define infraredHelpers_cpp
#include <Arduino.h>						//Standard Arduino library

#include <infraredHelpers.h>

//Shared by both transmit and receive

infraredHelpers::infraredHelpers()	//Constructor function
{
}

infraredHelpers::~infraredHelpers()	//Destructor function
{
}
uint8_t infraredHelpers::getMinimumNumberOfSymbols()	//Maximum number of symbols
{
	return minimum_number_of_symbols_;
}
void infraredHelpers::setMaximumNumberOfSymbols(uint8_t symbols)				//Must be done before begin(), default is 48
{
	
	if(maximum_number_of_symbols_ > minimum_number_of_symbols_)					//There is a minimum value on ESP32/RMT
	{
		maximum_number_of_symbols_ = symbols;
		if(debug_uart_ != nullptr)
		{
			debug_uart_->print(F("infraredHelpers maximum number of symbols: "));
			debug_uart_->println(maximum_message_length_);
		}
	}
}
uint8_t infraredHelpers::getMaximumMessageLength(uint8_t length)				//Get the maximum message length
{
	return maximum_message_length_;
}
void infraredHelpers::setMaximumMessageLength(uint8_t length)					//Set the maximum message length
{
	maximum_message_length_ = length;
	if(debug_uart_ != nullptr)
	{
		debug_uart_->print(F("infraredHelpers maximum message length: "));
		debug_uart_->println(maximum_message_length_);
	}
}

void infraredHelpers::debug(Stream &terminalStream)
{
	debug_uart_ = &terminalStream;		//Set the stream used for the terminal
	if(debug_uart_ != nullptr)
	{
		debug_uart_->print(F("infraredHelpers: debug enabled\r\n"));
	}
}

// Transmit helper

infraredTransmitHelper::infraredTransmitHelper()	//Constructor function
{
}
infraredTransmitHelper::~infraredTransmitHelper()	//Destructor function
{
}
bool infraredTransmitHelper::setCarrierFrequency(uint16_t frequency)				//Must be done before begin(), default is 56000
{
	return true;
}
bool infraredTransmitHelper::setDutyCycle(uint8_t duty, uint8_t transmitterIndex)	//Must be done before begin(), default is 50 and very unlikely to change
{
	return true;
}
bool infraredTransmitHelper::begin(uint8_t numberOfTransmitters)	//Set up transmitters
{
	return false;
}
bool infraredTransmitHelper::configureTxPin(uint8_t index, int8_t pin)				//Configure a pin for TX on the specified channel
{
	return false;
}
bool infraredTransmitHelper::addSymbol(uint8_t index, uint16_t duration0, uint8_t level0, uint16_t duration1, uint8_t level1)		//Add a symbol to the buffer for the specified transmitter channel
{
	return false;
}
bool infraredTransmitHelper::transmitSymbols(uint8_t transmitterIndex, bool wait)			//Transmit a buffer from the specified transmitter channel
{
	return false;
}
bool infraredTransmitHelper::transmitterBusy(uint8_t index)									//Used to check if busy before starting another transmission
{
	return false;
}

//Receive helper

infraredReceiveHelper::infraredReceiveHelper()	//Constructor function
{
}

infraredReceiveHelper::~infraredReceiveHelper()	//Destructor function
{
}
bool infraredReceiveHelper::begin(uint8_t numberOfReceivers)
{
	return false;
}
bool infraredReceiveHelper::configureRxPin(uint8_t index, int8_t pin, bool inverted)				//Configure a pin for RX on the current available channel
{
	return false;
}
uint8_t infraredReceiveHelper::receivedSymbolLevel0(uint8_t index, uint16_t symbolIndex)			//Getter for the symbol data
{
	return 0;
}
uint8_t infraredReceiveHelper::receivedSymbolLevel1(uint8_t index, uint16_t symbolIndex)			//Getter for the symbol data
{
	return 0;
}
uint16_t infraredReceiveHelper::receivedSymbolDuration0(uint8_t index, uint16_t symbolIndex)		//Getter for the symbol data
{
	return 0;
}
uint16_t infraredReceiveHelper::receivedSymbolDuration1(uint8_t index, uint16_t symbolIndex)		//Getter for the symbol data
{
	return 0;
}
uint8_t infraredReceiveHelper::getNumberOfReceivedSymbols(uint8_t index)
{
	if(index < number_of_receivers_)
	{
		return number_of_received_symbols_[index];
	}
	return 0;
}
void infraredReceiveHelper::resume(uint8_t index)																		//Resume reception on a specific channel
{
}


#endif