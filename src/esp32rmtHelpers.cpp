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
#ifndef esp32rmtHelpers_cpp
#define esp32rmtHelpers_cpp
#if defined ESP32
#include <esp32rmtHelpers.h>

esp32rmtTransmitHelper::esp32rmtTransmitHelper()	//Constructor function
{
}

esp32rmtTransmitHelper::~esp32rmtTransmitHelper()	//Destructor function
{
}
/*
 *
 * Non-class callback, ugh
 *
 */
bool esp32rmtTransmitHelperTxDoneCallback(rmt_channel_handle_t channel, const rmt_tx_done_event_data_t *edata, void *user_data)
{
	*(uint8_t *)(user_data) = 0;	//Reset the symbol count, which shows this channel as free
	return false;
}
/*
 *
 *	Initialise one or more transmitters
 *
 */
bool esp32rmtTransmitHelper::begin(uint8_t numberOfTransmitters)
{
	bool initialisation_success_ = true;
	number_of_transmitters_ = numberOfTransmitters;											//Record the number of transmitters
	infrared_transmitter_handle_ = new rmt_channel_handle_t[number_of_transmitters_];		//Create array of RMT handle(s)
	infrared_transmitter_config_ = new rmt_tx_channel_config_t[number_of_transmitters_];	//Create array of RMT configuration(s)
	symbols_to_transmit_ = new rmt_symbol_word_t*[number_of_transmitters_];					//Create array of symbol buffer(s)
	number_of_symbols_to_transmit_ = new uint8_t[number_of_transmitters_];					//Create array of symbol buffer length(s)
	#if defined SOC_RMT_TX_CANDIDATES_PER_GROUP
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: Creating TX helper for %u channels, candidate RMT TX channels: %u\r\n"), numberOfTransmitters, SOC_RMT_TX_CANDIDATES_PER_GROUP);
		}
	#else
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: Creating TX helper for %u channels\r\n"), numberOfTransmitters);
		}
	#endif
	#if defined SOC_RMT_MEM_WORDS_PER_CHANNEL
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: Each RMT channel has %u words memory available\r\n"), SOC_RMT_MEM_WORDS_PER_CHANNEL);
		}
	#endif
	if(rmt_new_copy_encoder(&copy_encoder_config_, &copy_encoder_) != ESP_OK)				//Initialise the copy encoder
	{
		if(debug_uart_ != nullptr)
		{
			debug_uart_->println(F("esp32rmtTransmitHelper: failed to create copy encoder"));
		}
		initialisation_success_ = false;
	}
	else
	{
		if(debug_uart_ != nullptr)
		{
			debug_uart_->println(F("esp32rmtTransmitHelper: created copy encoder"));
		}
	}
	for(uint8_t index = 0; index < number_of_transmitters_; index++)
	{
		symbols_to_transmit_[index] = new rmt_symbol_word_t[getMaximumNumberOfSymbols()];
		number_of_symbols_to_transmit_[index] = 0;
	}
	return initialisation_success_;
}

bool esp32rmtTransmitHelper::setCarrierFrequency(uint16_t frequency)				//Must be done before begin(), default is 56000
{
	global_transmitter_config_.frequency_hz = frequency;
	return true;
}

bool esp32rmtTransmitHelper::setDutyCycle(uint8_t duty, uint8_t transmitterIndex)	//Must be done before begin(), default is 50 and very unlikely to change
{
	if(duty > 4 && duty < 76)
	{
		global_transmitter_config_.duty_cycle = float(duty)/100.0;
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: duty cycle set at %u%%\r\n"), duty);
		}
		return true;
	}
	return false;
}

bool esp32rmtTransmitHelper::configureTxPin(uint8_t index, int8_t pin)
{
	infrared_transmitter_config_[index] = {
		.gpio_num = static_cast<gpio_num_t>(pin),
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000, // 1MHz resolution, 1 tick = 1us
		.mem_block_symbols = getMaximumNumberOfSymbols(),
		.trans_queue_depth = 4,
	};
	infrared_transmitter_config_[index].flags = {
		#if CONFIG_IDF_TARGET_ESP32C3
			.with_dma = false,
		#else
			.with_dma = true,
		#endif
	};
	if(rmt_new_tx_channel(&infrared_transmitter_config_[index], &infrared_transmitter_handle_[index]) == ESP_OK)
	{
		rmt_tx_event_callbacks_t transmit_callbacks_ = {
			.on_trans_done = esp32rmtTransmitHelperTxDoneCallback
		};
		rmt_tx_register_event_callbacks(infrared_transmitter_handle_[index], &transmit_callbacks_, &number_of_symbols_to_transmit_[index]);
		rmt_apply_carrier(infrared_transmitter_handle_[index], &global_transmitter_config_);
		rmt_enable(infrared_transmitter_handle_[index]);
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: configured pin %u as transmitter %u at %.2fKHz %u%% duty cycle\r\n"), pin, index, float(global_transmitter_config_.frequency_hz/1000), uint8_t(global_transmitter_config_.duty_cycle*100));
		}
		return true;
	}
	else
	{
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: failed to configure pin %u for TX\r\n"), pin);
		}
	}
	return false;
}

bool esp32rmtTransmitHelper::addSymbol(uint8_t index, uint16_t duration0, uint8_t level0, uint16_t duration1, uint8_t level1)
{
	if(number_of_symbols_to_transmit_[index] < getMaximumNumberOfSymbols())
	{
		symbols_to_transmit_[index][number_of_symbols_to_transmit_[index]].duration0 = duration0;
		symbols_to_transmit_[index][number_of_symbols_to_transmit_[index]].level0 = level0;
		symbols_to_transmit_[index][number_of_symbols_to_transmit_[index]].duration1 = duration1;
		symbols_to_transmit_[index][number_of_symbols_to_transmit_[index]].level1 = level1;
		number_of_symbols_to_transmit_[index] = number_of_symbols_to_transmit_[index] + 1;
		return true;
	}
	return false;
}
bool esp32rmtTransmitHelper::transmitSymbols(uint8_t transmitterIndex, bool wait)	//Transmit a buffer from the specified transmitter channel
{
	if(debug_uart_ != nullptr)
	{
		debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: sending %u symbols on channel %u wait = %u\r\n"), number_of_symbols_to_transmit_[transmitterIndex], transmitterIndex, wait);
		for(uint8_t index = 0; index < number_of_symbols_to_transmit_[transmitterIndex]; index++)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: symbol %02u - %s:%04u/%s:%04u\r\n"), index, (symbols_to_transmit_[transmitterIndex][index].level0 == 0 ? "Off":"On"), symbols_to_transmit_[transmitterIndex][index].duration0, (symbols_to_transmit_[transmitterIndex][index].level1 == 0 ? "Off":"On"), symbols_to_transmit_[transmitterIndex][index].duration1);
		}
	}
	uint32_t sendStart = micros();
	esp_err_t result = rmt_transmit(infrared_transmitter_handle_[transmitterIndex], copy_encoder_, symbols_to_transmit_[transmitterIndex], number_of_symbols_to_transmit_[transmitterIndex]*sizeof(rmt_symbol_word_t), &event_transmitter_config_);	
	if(wait == true)	//Block until transmitted
	{
		rmt_tx_wait_all_done(infrared_transmitter_handle_[transmitterIndex], 1000);
	}
	uint32_t sendEnd = micros();
	if(result == ESP_OK)
	{
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: queued data for transmitter %u in %u microseconds \r\n"), transmitterIndex, sendEnd - sendStart);
		}
		return true;
	}
	else
	{
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: failed to transmit from transmitter %u\r\n"), transmitterIndex);
		}
	}
	return false;
}
bool esp32rmtTransmitHelper::transmitterBusy(uint8_t index)		//Used to check if busy before starting another transmission
{
	return number_of_symbols_to_transmit_[index] != 0;
}
esp32rmtTransmitHelper transmitHelper;	//Create an instance of the class, as only one is practically usable at a time despite not being a singleton

esp32rmtReceiveHelper::esp32rmtReceiveHelper()		//Constructor function
{
}
esp32rmtReceiveHelper::~esp32rmtReceiveHelper()		//Destructor function
{
}
/*
 *
 *	Initialise one or more receivers
 *
 */
bool esp32rmtReceiveHelper::begin(uint8_t numberOfReceivers)
{
	bool initialisation_success_ = true;
	number_of_receivers_ = numberOfReceivers;											//Record the number of receivers
	infrared_receiver_handle_ = new rmt_channel_handle_t[number_of_receivers_];			//Create array of RMT handle(s)
	infrared_receiver_config_ = new rmt_rx_channel_config_t[number_of_receivers_];		//Create array of RMT configuration(s)	
	received_symbols_ = new rmt_symbol_word_t*[number_of_receivers_];					//Create array of symbol buffer(s)
	number_of_received_symbols_ = new uint8_t[number_of_receivers_];					//Create array of symbol buffer length(s)
	for(uint8_t index = 0; index < number_of_receivers_; index++)
	{
		received_symbols_[index] = new rmt_symbol_word_t[getMaximumNumberOfSymbols()];	//Create symbol buffers
		number_of_received_symbols_[index] = 0;											//Set received buffer length to zero
	}
	#if defined SOC_RMT_RX_CANDIDATES_PER_GROUP
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtTransmitHelper: candidate RMT RX channels: %u"), SOC_RMT_RX_CANDIDATES_PER_GROUP);
		}
	#endif
	return initialisation_success_;
}
bool esp32rmtReceiverHelperRxDoneCallback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
	*(uint8_t *)(user_data) = edata->num_symbols;
	return false;
}
bool esp32rmtReceiveHelper::configureRxPin(uint8_t index, int8_t pin, bool inverted)
{
	infrared_receiver_config_[index] = {
		.gpio_num = static_cast<gpio_num_t>(pin),
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = getMaximumNumberOfSymbols(),
	};
	infrared_receiver_config_[index].flags = {
		.invert_in = inverted,
		.with_dma = false,
	};
	if(rmt_new_rx_channel(&infrared_receiver_config_[index], &infrared_receiver_handle_[index]) == ESP_OK)
	{
		rmt_rx_event_callbacks_t receive_callbacks_ = {
			.on_recv_done = esp32rmtReceiverHelperRxDoneCallback
		};
		rmt_rx_register_event_callbacks(infrared_receiver_handle_[index], &receive_callbacks_, &number_of_received_symbols_[index]);
		rmt_enable(infrared_receiver_handle_[index]);
		rmt_receive(infrared_receiver_handle_[index], received_symbols_[index], getMaximumNumberOfSymbols()*sizeof(rmt_symbol_word_t), &global_receiver_config_);
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtReceiveHelper: configured pin %u for RX\r\n"), pin);
		}
		return true;
	}
	else
	{
		if(debug_uart_ != nullptr)
		{
			debug_uart_->printf_P(PSTR("esp32rmtReceiveHelper: failed to configure pin %u for RX\r\n"), pin);
		}
	}
	return false;
}
uint8_t esp32rmtReceiveHelper::receivedSymbolLevel0(uint8_t index, uint16_t symbolIndex)
{
	return received_symbols_[index][symbolIndex].level0;
}
uint8_t esp32rmtReceiveHelper::receivedSymbolLevel1(uint8_t index, uint16_t symbolIndex)
{
	return received_symbols_[index][symbolIndex].level1;
}
uint16_t esp32rmtReceiveHelper::receivedSymbolDuration0(uint8_t index, uint16_t symbolIndex)
{
	return received_symbols_[index][symbolIndex].duration0;
}
uint16_t esp32rmtReceiveHelper::receivedSymbolDuration1(uint8_t index, uint16_t symbolIndex)
{
	return received_symbols_[index][symbolIndex].duration1;
}

void esp32rmtReceiveHelper::resume(uint8_t index)
{
	number_of_received_symbols_[index] = 0;
	rmt_receive(infrared_receiver_handle_[index], received_symbols_[index], getMaximumNumberOfSymbols()*sizeof(rmt_symbol_word_t), &global_receiver_config_);
}
esp32rmtReceiveHelper receiveHelper;	//Create an instance of the class, as only one is practically usable at a time despite not being a singleton
#endif
#endif