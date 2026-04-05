#ifndef avrInfraredHelpers_cpp
#define avrInfraredHelpers_cpp
#if defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_NANO
#include <avrInfraredHelpers.h>

//Transmit helpers
avrInfraredTransmitHelper::avrInfraredTransmitHelper()	//Constructor function
{
}
avrInfraredTransmitHelper::~avrInfraredTransmitHelper()	//Destructor function
{
}
bool avrInfraredTransmitHelper::setCarrierFrequency(uint16_t frequency)				//Must be done before begin(), default is 56000
{
	return true;
}
bool avrInfraredTransmitHelper::setDutyCycle(uint8_t duty, uint8_t transmitterIndex)	//Must be done before begin(), default is 50 and very unlikely to change
{
	return true;
}
bool avrInfraredTransmitHelper::begin(uint8_t numberOfTransmitters)	//Set up transmitters
{
	return false;
}
bool avrInfraredTransmitHelper::configureTxPin(uint8_t transmitterIndex, int8_t pin)				//Configure a pin for TX on the specified channel
{
	return false;
}
bool avrInfraredTransmitHelper::addSymbol(uint8_t transmitterIndex, uint16_t duration0, uint8_t level0, uint16_t duration1, uint8_t level1)		//Add a symbol to the buffer for the specified transmitter channel
{
	return false;
}
bool avrInfraredTransmitHelper::transmitSymbols(uint8_t transmitterIndex, bool wait)			//Transmit a buffer from the specified transmitter channel
{
	return false;
}
bool avrInfraredTransmitHelper::transmitterBusy(uint8_t transmitterIndex)									//Used to check if busy before starting another transmission
{
	return false;
}
avrInfraredTransmitHelper transmitHelper;	//Create an instance of the class, as only one is practically usable at a time despite not being a singleton

//Receive helpers
avrInfraredReceiveHelper::avrInfraredReceiveHelper()	//Constructor function
{
	setMaximumNumberOfSymbols(getMinimumNumberOfSymbols());
}
avrInfraredReceiveHelper::~avrInfraredReceiveHelper()	//Destructor function
{
}
bool avrInfraredReceiveHelper::begin(uint8_t numberOfReceivers)
{
	bool initialisation_success_ = true;
	number_of_receivers_ = numberOfReceivers;														//Record the number of receivers
	received_symbols_ = new volatile uint16_t*[number_of_receivers_];										//Create array of symbol buffer(s)
	waiting_for_symbols_ = new volatile bool[number_of_receivers_];
	number_of_received_symbols_ = new volatile uint8_t[number_of_receivers_];								//Create array of symbol buffer length(s)
	last_edge_ = new volatile uint32_t[number_of_receivers_];												//Edge timers
	current_edge_index_ = new volatile uint16_t[number_of_receivers_];												//Edge timer index
	for(uint8_t receiverIndex = 0; receiverIndex < number_of_receivers_; receiverIndex++)
	{
		received_symbols_[receiverIndex] = new uint16_t[getMaximumNumberOfSymbols()*2];						//Create symbol buffers
		for(uint8_t symbolIndex = 0; symbolIndex < getMaximumNumberOfSymbols()*2; symbolIndex++)
		{
			received_symbols_[receiverIndex][symbolIndex] = 0;												//Blank out the buffer, just in case
		}
		number_of_received_symbols_[receiverIndex] = 0;														//Set received buffer length to zero
		last_edge_[receiverIndex] = 0;																		//Clear last edge time
		current_edge_index_[receiverIndex] = 0;																	//Reset symbol index
		waiting_for_symbols_[receiverIndex] = true;															//Be ready to accept symbols
	}
	if(debug_uart_ != nullptr)
	{
		debug_uart_->print(F("avrInfraredReceiveHelper: configuring "));
		debug_uart_->print(numberOfReceivers);
		debug_uart_->print(F(" receiver(s) for up to "));
		debug_uart_->print(getMaximumNumberOfSymbols());
		debug_uart_->println(F(" symbols"));
	}
	return initialisation_success_;
}
inline void avrInfraredReceiveHelperIsr(uint8_t receiverIndex)
{
	if(receiveHelper.waiting_for_symbols_[receiverIndex] == true)
	{
		receiveHelper.last_edge_[receiverIndex] = micros();
		receiveHelper.received_symbols_[receiverIndex][receiveHelper.current_edge_index_[receiverIndex]++] = (receiveHelper.last_edge_[receiverIndex] - receiveHelper.last_edge_[receiverIndex]);	//Divide by 8
		if(receiveHelper.current_edge_index_[receiverIndex] == receiveHelper.getMaximumNumberOfSymbols()*2) //Stop accepting timings because we're out of array!
		{
			receiveHelper.waiting_for_symbols_[receiverIndex] = false;
			Serial.println(F("avrInfraredReceiveHelper: timing buffer overflow"));
		}
	}
}
inline void avrInfraredReceiveHelperIsr0()
{
	avrInfraredReceiveHelperIsr(0);
}
inline void avrInfraredReceiveHelperIsr1()
{
	avrInfraredReceiveHelperIsr(1);
}
bool avrInfraredReceiveHelper::configureRxPin(uint8_t receiverIndex, int8_t pin, bool inverted)				//Configure a pin for RX on the current available channel
{
	if(receiverIndex < 2)
	{
		pinMode(pin, INPUT);
		delay(500);
		if(digitalRead(pin) == false)
		{
			if(debug_uart_ != nullptr)
			{
				debug_uart_->print(F("avrInfraredReceiveHelper: warning pin "));
				debug_uart_->print(pin);
				debug_uart_->print(F(" not connected to receiver\r\n"));
			}
			return true;
		}
		switch (receiverIndex)
		{
			case 0:
				attachInterrupt(digitalPinToInterrupt(pin), avrInfraredReceiveHelperIsr0, CHANGE);
			break;
			case 1:
				attachInterrupt(digitalPinToInterrupt(pin), avrInfraredReceiveHelperIsr1, CHANGE);
			break;
			default:
				return false;
			break;
		}
		waiting_for_symbols_[receiverIndex] = true;
		if(true)
		{
			if(debug_uart_ != nullptr)
			{
				debug_uart_->print(F("avrInfraredReceiveHelper: configured pin "));
				debug_uart_->print(pin);
				debug_uart_->print(F(" for RX\r\n"));
			}
			return true;
		}
		else
		{
			if(debug_uart_ != nullptr)
			{
				debug_uart_->print(F("avrInfraredReceiveHelper: failed to configure pin "));
				debug_uart_->print(pin);
				debug_uart_->print(F(" for RX\r\n"));
			}
		}
	}
	return false;
}
uint8_t avrInfraredReceiveHelper::receivedSymbolLevel0(uint8_t receiverIndex, uint16_t symbolIndex)			//Getter for the symbol data
{
	return 1;	//Not recording this so assume level0 is high
	//return (symbolIndex & 0x0001) == 0;	//Assume even durations are low
}
uint8_t avrInfraredReceiveHelper::receivedSymbolLevel1(uint8_t receiverIndex, uint16_t symbolIndex)			//Getter for the symbol data
{
	return 0;	//Not recording this so assume level1 is low
	//return (symbolIndex & 0x0001) != 0;	//Assume odd durations are high
}
uint16_t avrInfraredReceiveHelper::receivedSymbolDuration0(uint8_t receiverIndex, uint16_t symbolIndex)		//Getter for the symbol data
{
	return uint16_t(received_symbols_[receiverIndex][symbolIndex<<1]);	//Even entries are duration0
}
uint16_t avrInfraredReceiveHelper::receivedSymbolDuration1(uint8_t receiverIndex, uint16_t symbolIndex)		//Getter for the symbol data
{
	return uint16_t(received_symbols_[receiverIndex][1+(symbolIndex<<1)]);	//Odd entries are duration1
}
uint8_t avrInfraredReceiveHelper::getNumberOfReceivedSymbols(uint8_t receiverIndex)
{
	if(receiverIndex < number_of_receivers_)
	{
		if(waiting_for_symbols_[receiverIndex] == true)
		{
			//if(current_edge_index_[receiverIndex] > 0 && micros() - last_edge_[receiverIndex] > symbol_timeout_)			//It's been a while since an edge so we've received some IR symbols
			if(current_edge_index_[receiverIndex] > 0)			//It's been a while since an edge so we've received some IR symbols
			{
				if(micros() - last_edge_[receiverIndex] > symbol_timeout_)
				{
					waiting_for_symbols_[receiverIndex] = false;
					if((current_edge_index_[receiverIndex] & 0x0001) == 0)														//It's an even index
					{
						received_symbols_[receiverIndex][current_edge_index_[receiverIndex]++] = 0;	//The last level is always an even index with level low and duration zero
						if(debug_uart_ != nullptr)
						{
							debug_uart_->print(F("avrInfraredReceiveHelper: filling in zero off time for symbol "));
							debug_uart_->println(current_edge_index_[receiverIndex]>>1);
						}
					}
					number_of_received_symbols_[receiverIndex] = current_edge_index_[receiverIndex]>>1;							//Update symbol count from symbol buffer index/2
					current_edge_index_[receiverIndex] = 0;
					if(debug_uart_ != nullptr)
					{
						debug_uart_->print(F("avrInfraredReceiveHelper: partially full buffer on channel "));
						debug_uart_->print(receiverIndex);
						debug_uart_->print(F(" with "));
						debug_uart_->print(number_of_received_symbols_[receiverIndex]);
						debug_uart_->println(F(" symbols"));
					}
					return number_of_received_symbols_[receiverIndex];															//Inform the caller there's a usefully filled symbol buffer
				}
				else
				{
					/*
					if(debug_uart_ != nullptr)
					{
						debug_uart_->print(F("avrInfraredReceiveHelper: waiting for timeout "));
						debug_uart_->println(micros() - last_edge_[receiverIndex]);
					}
					uint32_t now = millis();
					while(millis() - now < 3000)
					{
					}
					*/
				}
			}
		}
		else
		{
			if(number_of_received_symbols_[receiverIndex] > 0)	//We've stopped looking for new symbols
			{
				return number_of_received_symbols_[receiverIndex];
			}
			else if(current_edge_index_[receiverIndex] > 0)	//There is a completely full buffer that stopped early to process
			{
				number_of_received_symbols_[receiverIndex] = current_edge_index_[receiverIndex]>>1;							//Update symbol count from symbol buffer index/2
				current_edge_index_[receiverIndex] = 0;
				if(debug_uart_ != nullptr)
				{
					debug_uart_->print(F("avrInfraredReceiveHelper: full buffer on channel "));
					debug_uart_->print(receiverIndex);
					debug_uart_->print(F(" with "));
					debug_uart_->print(number_of_received_symbols_[receiverIndex]);
					debug_uart_->println(F(" symbols"));
				}
				return number_of_received_symbols_[receiverIndex];															//Inform the caller there's a usefully filled symbol buffer
			}
		}
	}
	return 0;
}
void avrInfraredReceiveHelper::resume(uint8_t receiverIndex)
{
	if(debug_uart_ != nullptr)
	{
		debug_uart_->print(F("avrInfraredReceiveHelper: resuming reception on channel "));
		debug_uart_->println(receiverIndex);
	}
	number_of_received_symbols_[receiverIndex] = 0;
	last_edge_[receiverIndex] = 0;
	current_edge_index_[receiverIndex] = 0;
	waiting_for_symbols_[receiverIndex] = true;
}
void avrInfraredReceiveHelper::setSymbolTimeout(uint16_t timeout)
{
	if(debug_uart_ != nullptr)
	{
		debug_uart_->print(F("avrInfraredReceiveHelper: set symbol timeout to "));
		debug_uart_->print(timeout);
		debug_uart_->println(F("us"));
	}
	symbol_timeout_ = timeout;
}
avrInfraredReceiveHelper receiveHelper;	//Create an instance of the class, as only one is practically usable at a time despite not being a singleton

#endif
#endif
