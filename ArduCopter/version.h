#pragma once

#include "AP_Common/AP_FWVersion.h"

#define THISFIRMWARE "APM:Copter V3.6-dev"

// the following line is parsed by the autotest scripts
#define FIRMWARE_VERSION 3,6,0,FIRMWARE_VERSION_TYPE_DEV

#define FW_MAJOR 3
#define FW_MINOR 6
#define FW_PATCH 0
#define FW_TYPE FIRMWARE_VERSION_TYPE_DEV

extern const AP_FWVersion fwver;
