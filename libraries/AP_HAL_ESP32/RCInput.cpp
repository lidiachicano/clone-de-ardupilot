/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Code by David "Buzz" Bussenschutt and others
 */
#include "RCInput.h"
#include <stdio.h>


//#include "hal.h"
//#include "hwdef/common/ppm.h"
//#if CONFIG_HAL_BOARD == HAL_BOARD_ESP32

// #if HAL_WITH_IO_MCU
// #include <AP_BoardConfig/AP_BoardConfig.h>
// #include <AP_IOMCU/AP_IOMCU.h>
// extern AP_IOMCU iomcu;
// #endif

#include <AP_Math/AP_Math.h>

#ifndef HAL_NO_UARTDRIVER
#include <GCS_MAVLink/GCS.h>
#endif

#define SIG_DETECT_TIMEOUT_US 500000
using namespace ESP32;
extern const AP_HAL::HAL& hal;
void RCInput::init()
{
   printf("RCInput::init()\n");
// #if HAL_USE_ICU == TRUE
//     //attach timer channel on which the signal will be received
//     sig_reader.attach_capture_timer(&RCIN_ICU_TIMER, RCIN_ICU_CHANNEL, STM32_RCIN_DMA_STREAM, STM32_RCIN_DMA_CHANNEL);
//     rcin_prot.init();
// #endif

//#if HAL_USE_EICU == TRUE
    sig_reader.init();
    rcin_prot.init();
//#endif

    _init = true;
}

bool RCInput::new_input()
{
    if (!_init) {
        return false;
    }
    if (!rcin_mutex.take_nonblocking()) {
        return false;
    }
    bool valid = _rcin_timestamp_last_signal != _last_read;

    _last_read = _rcin_timestamp_last_signal;
    rcin_mutex.give();

#if HAL_RCINPUT_WITH_AP_RADIO
    if (!_radio_init) {
        _radio_init = true;
        radio = AP_Radio::instance();
        if (radio) {
            radio->init();
        }
    }
#endif    
    return valid;
}

uint8_t RCInput::num_channels()
{
    if (!_init) {
        return 0;
    }
    return _num_channels;
}

uint16_t RCInput::read(uint8_t channel)
{
    if (!_init || (channel >= MIN(RC_INPUT_MAX_CHANNELS, _num_channels))) {
        return 0;
    }
    rcin_mutex.take(HAL_SEMAPHORE_BLOCK_FOREVER);
    uint16_t v = _rc_values[channel];
    rcin_mutex.give();
//#if HAL_RCINPUT_WITH_AP_RADIO
//    if (radio && channel == 0) {
//        // hook to allow for update of radio on main thread, for mavlink sends
//        radio->update();
//    }
//#endif
    return v;
}

uint8_t RCInput::read(uint16_t* periods, uint8_t len)
{
    if (!_init) {
        return false;
    }
 
    if (len > RC_INPUT_MAX_CHANNELS) {
        len = RC_INPUT_MAX_CHANNELS;
    }
    for (uint8_t i = 0; i < len; i++){
        periods[i] = read(i);
    }
    return len;
}

void RCInput::_timer_tick(void)
{

	//printf("RCInput _timer_tick debug");

    if (!_init) { //BUZZ PUT THIS BACK HACK
        return;
    }

   // #if HAL_USE_EICU == TRUE
        uint32_t width_s0, width_s1;
        while(sig_reader.read(width_s0, width_s1)) {
            rcin_prot.process_pulse(width_s0, width_s1);
        }
   // #endif

    //#ifndef HAL_NO_UARTDRIVER
        const char *rc_protocol = nullptr;
    //#endif

    //#if HAL_USE_ICU == TRUE || HAL_USE_EICU == TRUE
        if (rcin_prot.new_input()) {
            rcin_mutex.take(HAL_SEMAPHORE_BLOCK_FOREVER);
            _rcin_timestamp_last_signal = AP_HAL::micros();
            _num_channels = rcin_prot.num_channels();
            _num_channels = MIN(_num_channels, RC_INPUT_MAX_CHANNELS);
            for (uint8_t i=0; i<_num_channels; i++) {
                _rc_values[i] = rcin_prot.read(i);
            }
            rcin_mutex.give();
    //#ifndef HAL_NO_UARTDRIVER
            rc_protocol = rcin_prot.protocol_name();
    //#endif
    }
//#endif

    //#ifndef HAL_NO_UARTDRIVER
        if (rc_protocol && rc_protocol != last_protocol) {
            last_protocol = rc_protocol;
            gcs().send_text(MAV_SEVERITY_DEBUG, "RCInput: decoding %s", last_protocol);
        }
    //#endif

    // note, we rely on the vehicle code checking new_input()
    // and a timeout for the last valid input to handle failsafe
}

/*
  start a bind operation, if supported
 */
bool RCInput::rc_bind(int dsmMode)
{
  // not impl
    return true;
}
//#endif //#if CONFIG_HAL_BOARD == HAL_BOARD_ESP32
