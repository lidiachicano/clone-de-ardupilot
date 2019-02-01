#include "Copter.h"

/*
 * Init and run calls for stabilize flight mode
 */

// stabilize_init - initialise stabilize controller
bool Copter::ModeHoldUltra::init(bool ignore_checks)
{
    // Ronny: Hier kann ich die Klasse initialisieren
	// und angeben, was zum einschalten notwendig ist.
	// Wenn false zurück gegeben wird, dann wird der Mode nicht eingeschaltet
	// if landed and the mode we're switching from does not have manual throttle and the throttle stick is too high
    /*if (motors->armed() && ap.land_complete && !copter.flightmode->has_manual_throttle() &&
            (get_pilot_desired_throttle(channel_throttle->get_control_in()) > get_non_takeoff_throttle())) {
        return false;
    }
	*/
	
	// Hie könnte man einen 1kHz timer einschalten:
	// hal.scheduler->register_timer_process(AP_HAL_MEMBERPROC(&AP_Baro_MS5611::_update));
    return true;
}

// stabilize_run - runs the main stabilize controller
// should be called at 100hz or more
void Copter::ModeHoldUltra::run()
{
    float target_roll, target_pitch;
    float target_yaw_rate;
    float pilot_throttle_scaled;

    // if not armed set throttle to zero and exit immediately
	/*
    if (!motors->armed() || ap.throttle_zero || !motors->get_interlock()) {
        zero_throttle_and_relax_ac();
        return;
    }
	*/

	/*
    // clear landing flag
    set_land_complete(false);

    motors->set_desired_spool_state(AP_Motors::DESIRED_THROTTLE_UNLIMITED);

    // apply SIMPLE mode transform to pilot inputs
    update_simple_mode();
	*/
	
	// Piloten-Eingabe auslesen:
	/*

    // convert pilot input to lean angles
    get_pilot_desired_lean_angles(target_roll, target_pitch, copter.aparm.angle_max, copter.aparm.angle_max);
	
    // get pilot's desired yaw rate
    target_yaw_rate = get_pilot_desired_yaw_rate(channel_yaw->get_control_in());

    // get pilot's desired throttle
    pilot_throttle_scaled = get_pilot_desired_throttle(channel_throttle->get_control_in());
	*/
	
	// Ronny: UND HIER WIRD DIE STEUERUNG ÜBERNOMMEN DIE ICH ÜBERNEHMEN MUSS !!!!!!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
    // call attitude controller
	// Lage in allen drei Achsen:
    // attitude_control->input_euler_angle_roll_pitch_euler_rate_yaw(target_roll, target_pitch, target_yaw_rate);

    // body-frame rate controller is run directly from 100hz loop

    // output pilot's throttle
	// Gas:
    // attitude_control->set_throttle_out(pilot_throttle_scaled, true, g.throttle_filt);
}
