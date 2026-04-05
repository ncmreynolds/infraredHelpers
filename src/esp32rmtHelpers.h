/*
 *	An Arduino library to provide 'helpers' for the RMT peripheral on ESP32 when sending remote-control style infrared signals
 *
 *	Mostly it is used in the following libraries that are for making Laser-Tag equipment
 *
 *	https://github.com/ncmreynolds/milesTag
 *	https://github.com/ncmreynolds/WoW
 *	https://github.com/ncmreynolds/DoT (closed source)
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/esp32rmtHelpers/LICENSE for full license
 *
 */
#ifndef esp32rmtHelpers_h
#define esp32rmtHelpers_h
#if defined ESP32
#include <Arduino.h>						//Standard Arduino library
#include <infraredHelpers.h>

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

class esp32rmtTransmitHelper : public infraredHelpers, public infraredTransmitHelper {
	public:
		esp32rmtTransmitHelper();												//Constructor function
		~esp32rmtTransmitHelper();												//Destructor function
		bool setCarrierFrequency(uint16_t frequency);							//Must be done before begin(), default is 56000
		bool setDutyCycle(uint8_t duty, uint8_t transmitterIndex = 0);			//Must be done before begin(), default is 50 and very unlikely to change
		bool begin(uint8_t numberOfTransmitters = 1);							//Initialise one or more transmitters
		bool configureTxPin(uint8_t index, int8_t pin);							//Configure a pin for TX on the current available channel
		bool addSymbol(uint8_t index, uint16_t duration0, uint8_t level0,		//Add a symbol to the buffer for the specified transmitter channel
			uint16_t duration1, uint8_t level1);
		bool transmitSymbols(uint8_t transmitterIndex,							//Transmit a buffer from the specified transmitter channel
				bool wait = false);
		bool transmitterBusy(uint8_t index);									//Used to check if busy before starting another transmission

	protected:

	private:
		//Global RMT settings
		rmt_carrier_config_t global_transmitter_config_ = {						//Global config across all receivers
			.frequency_hz = 56000,
			.duty_cycle = 0.50,
			//.flags = {
			//	.polarity_active_low = 0
			//}
		};
		rmt_transmit_config_t event_transmitter_config_ = {
			//.eot_level = 0,														//Drive pin low at end
			//.loop_count = 0,													//Do not loop
		};
		rmt_encoder_t *copy_encoder_;											//We will use a 'copy encoder' and do all encoding ourselves
		rmt_copy_encoder_config_t copy_encoder_config_ = {};					//The copy encoder supports no configuration, but must exist
		rmt_channel_handle_t* infrared_transmitter_handle_ = nullptr;			//RMT transmitter channels
		rmt_tx_channel_config_t* infrared_transmitter_config_ = nullptr;		//The RMT configuration for the transmitter(s)
		//Symbol buffers
		rmt_symbol_word_t** symbols_to_transmit_ = nullptr;						//Symbol buffers

};
extern esp32rmtTransmitHelper transmitHelper;	//Create an instance of the class, as only one is practically usable at a time despite not being a singleton

class esp32rmtReceiveHelper : public infraredHelpers, public infraredReceiveHelper {
	public:
		esp32rmtReceiveHelper();													//Constructor function
		~esp32rmtReceiveHelper();													//Destructor function
		bool begin(uint8_t numberOfReceivers = 1);									//Initialise one or more receivers
		bool configureRxPin(uint8_t index, int8_t pin, bool inverted = true);		//Configure a pin for RX on the current available channel
		uint8_t receivedSymbolLevel0(uint8_t index, uint16_t symbolIndex);			//Getters for the symbol data
		uint8_t receivedSymbolLevel1(uint8_t index, uint16_t symbolIndex);
		uint16_t receivedSymbolDuration0(uint8_t index, uint16_t symbolIndex);
		uint16_t receivedSymbolDuration1(uint8_t index, uint16_t symbolIndex);
		void resume(uint8_t index);													//Resume reception on a specific channel
		
	protected:

	private:
		//Global RMT settings
		rmt_receive_config_t global_receiver_config_ = {						//Global config across all receivers
			.signal_range_min_ns = 2000,										//Actually 600us but 2us is the smallest acceptable value in the SDK
			.signal_range_max_ns = 2800000,										//Actually 2400us but allow some margin
		};
		//Receiver RMT data
		rmt_rx_channel_config_t* infrared_receiver_config_ = nullptr;			//The RMT configuration for the receiver(s)
		rmt_channel_handle_t* infrared_receiver_handle_ = nullptr;				//RMT receiver channels
		//Symbol buffers
		rmt_symbol_word_t** received_symbols_;									//Symbol buffers
};
extern esp32rmtReceiveHelper receiveHelper;	//Create an instance of the class, as only one is practically usable at a time despite not being a singleton
#endif
#endif