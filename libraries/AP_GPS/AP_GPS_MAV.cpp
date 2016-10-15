// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//
//  MAVLINK GPS driver
//
#include "AP_GPS_MAV.h"
#include <stdint.h>

AP_GPS_MAV::AP_GPS_MAV(AP_GPS &_gps, AP_GPS::GPS_State &_state, AP_HAL::UARTDriver *_port) :
    AP_GPS_Backend(_gps, _state, _port)
{
    _new_data = false;
}

// Reading does nothing in this class; we simply return whether or not
// the latest reading has been consumed.  By calling this function we assume
// the caller is consuming the new data;
bool AP_GPS_MAV::read(void)
{
    if (_new_data) {
        _new_data = false;
        return true;
    }

    return false;
}

// handles an incoming mavlink message (HIL_GPS) and sets
// corresponding gps data appropriately;
void AP_GPS_MAV::handle_msg(const mavlink_message_t *msg)
{
    mavlink_gps_input_t packet;
    mavlink_msg_gps_input_decode(msg, &packet);

    uint16_t consider_flags = packet.ignore_flags ^ 0xffff;
    uint16_t have_alt    = consider_flags & GPS_INPUT_IGNORE_FLAG_ALT;
    uint16_t have_hdop   = consider_flags & GPS_INPUT_IGNORE_FLAG_HDOP;
    uint16_t have_vdop   = consider_flags & GPS_INPUT_IGNORE_FLAG_VDOP;
    uint16_t have_vel_h  = consider_flags & GPS_INPUT_IGNORE_FLAG_VEL_HORIZ;
    uint16_t have_vel_v  = consider_flags & GPS_INPUT_IGNORE_FLAG_VEL_VERT;
    uint16_t have_sa     = consider_flags & GPS_INPUT_IGNORE_FLAG_SPEED_ACCURACY;
    uint16_t have_ha     = consider_flags & GPS_INPUT_IGNORE_FLAG_HORIZONTAL_ACCURACY;
    uint16_t have_va     = consider_flags & GPS_INPUT_IGNORE_FLAG_VERTICAL_ACCURACY;

    state.time_week     = packet.time_week;
    state.time_week_ms  = packet.time_week_ms;
    state.status = (AP_GPS::GPS_Status)packet.fix_type;

    Location loc = {};
    loc.lat = packet.lat;
    loc.lng = packet.lon;
    if (have_alt) {
        loc.alt = packet.alt;
    }
    state.location = loc;
    state.location.options = 0;

    if (have_hdop) {
        state.hdop = packet.hdop * 10; //In centimeters
    }

    if (have_vdop) {
        state.vdop = packet.vdop * 10; //In centimeters
    }

    if (have_vel_h) {
        Vector3f vel(packet.vn, packet.ve, 0);
        if (have_vel_v)
            vel.z = packet.vd;

        state.velocity = vel;
        state.ground_course = wrap_360(degrees(atan2f(vel.y, vel.x)));
        state.ground_speed = norm(vel.x, vel.y);
    }

    if (have_sa) {
        state.speed_accuracy = packet.speed_accuracy;
        state.have_speed_accuracy = 1;
    }

    if (have_ha) {
        state.horizontal_accuracy = packet.horiz_accuracy;
        state.have_horizontal_accuracy = 1;
    }

    if (have_va) {
        state.vertical_accuracy = packet.vert_accuracy;
        state.have_vertical_accuracy = 1;
    }

    state.num_sats = packet.satellites_visible;

    state.last_gps_time_ms = AP_HAL::millis();

    _new_data = true;
}
