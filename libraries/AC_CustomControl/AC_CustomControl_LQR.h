#pragma once

#include "AC_CustomControl_Backend.h"

#ifndef CUSTOMCONTROL_LQR_ENABLED
    #define CUSTOMCONTROL_LQR_ENABLED AP_CUSTOMCONTROL_ENABLED
#endif

#if CUSTOMCONTROL_LQR_ENABLED

class AC_CustomControl_LQR : public AC_CustomControl_Backend {
public:
    AC_CustomControl_LQR(AC_CustomControl& frontend, AP_AHRS_View*& ahrs, AC_AttitudeControl*& att_control, AP_MotorsMulticopter*& motors, float dt);


    Vector3f update(void) override;
    void reset(void) override;
    // user settable parameters
    static const struct AP_Param::GroupInfo var_info[];

protected:
    // declare parameters here

    float _dt;

    float _integralX, _integralY, _integralZ;
    float _kimax_LQR = 0.08;

    AP_Float param1;
    AP_Float param2;
    AP_Float param3;
};

#endif
