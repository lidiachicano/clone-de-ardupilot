/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
//
// Unit tests for the AP_Math polygon code
//

#include <AP_Common/AP_Common.h>
#include <AP_Progmem/AP_Progmem.h>
#include <AP_Param/AP_Param.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>
#include <Filter/Filter.h>
#include <AP_ADC/AP_ADC.h>
#include <AP_Notify/AP_Notify.h>
#include <AP_InertialSensor/AP_InertialSensor.h>
#include <AP_GPS/AP_GPS.h>
#include <DataFlash/DataFlash.h>
#include <AP_Baro/AP_Baro.h>
#include <GCS_MAVLink/GCS_MAVLink.h>
#include <AP_Mission/AP_Mission.h>
#include <StorageManager/StorageManager.h>
#include <AP_Terrain/AP_Terrain.h>
#include <AP_Declination/AP_Declination.h>
#include <AP_Rally/AP_Rally.h>
#include <AP_OpticalFlow/AP_OpticalFlow.h>

#include <AP_HAL_AVR/AP_HAL_AVR.h>
#include <AP_HAL_SITL/AP_HAL_SITL.h>
#include <AP_HAL_Empty/AP_HAL_Empty.h>
#include <AP_HAL_Linux/AP_HAL_Linux.h>
#include <AP_AHRS/AP_AHRS.h>
#include <SITL/SITL.h>
#include <AP_NavEKF/AP_NavEKF.h>
#include <AP_Airspeed/AP_Airspeed.h>
#include <AP_Vehicle/AP_Vehicle.h>
#include <AP_ADC_AnalogSource/AP_ADC_AnalogSource.h>
#include <AP_Compass/AP_Compass.h>
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_RangeFinder/AP_RangeFinder.h>

const AP_HAL::HAL& hal = AP_HAL::get_HAL();

static const struct {
    Vector2f wp1, wp2, location;
    bool passed;
} test_points[] = {
    { Vector2f(-35.3647759314918f, 149.16265692810987f),
      Vector2f(-35.36279922658029f, 149.16352169591426f),
      Vector2f(-35.36214956969903f, 149.16461410046492f), true },
    { Vector2f(-35.36438601157189f, 149.16613916088568f),
      Vector2f(-35.364432558610254f, 149.16287313113048f),
      Vector2f(-35.36491510034746f, 149.16365837225004f), false },
    { Vector2f(0, 0),
      Vector2f(0, 1),
      Vector2f(0, 2), true },
    { Vector2f(0, 0),
      Vector2f(0, 2),
      Vector2f(0, 1), false },
    { Vector2f(0, 0),
      Vector2f(1, 0),
      Vector2f(2, 0), true },
    { Vector2f(0, 0),
      Vector2f(2, 0),
      Vector2f(1, 0), false },
    { Vector2f(0, 0),
      Vector2f(-1, 1),
      Vector2f(-2, 2), true },
};

static struct Location location_from_point(Vector2f pt)
{
    struct Location loc = {0};
    loc.lat = pt.x * 1.0e7f;
    loc.lng = pt.y * 1.0e7f;
    return loc;
}

static void test_passed_waypoint(void)
{
    hal.console->println("waypoint tests starting");
    for (uint8_t i=0; i<ARRAY_SIZE(test_points); i++) {
        struct Location loc = location_from_point(test_points[i].location);
        struct Location wp1 = location_from_point(test_points[i].wp1);
        struct Location wp2 = location_from_point(test_points[i].wp2);
        if (location_passed_point(loc, wp1, wp2) != test_points[i].passed) {
            hal.console->printf("Failed waypoint test %u\n", (unsigned)i);
            return;
        }
    }
    hal.console->println("waypoint tests OK");
}

static void test_one_offset(const struct Location &loc,
                            float ofs_north, float ofs_east,
                            float dist, float bearing)
{
    struct Location loc2;
    float dist2, bearing2;

    loc2 = loc;
    uint32_t t1 = hal.scheduler->micros();
    location_offset(loc2, ofs_north, ofs_east);
    hal.console->printf("location_offset took %u usec\n",
                        (unsigned)(hal.scheduler->micros() - t1));
    dist2 = get_distance(loc, loc2);
    bearing2 = get_bearing_cd(loc, loc2) * 0.01f;
    float brg_error = bearing2-bearing;
    if (brg_error > 180) {
        brg_error -= 360;
    } else if (brg_error < -180) {
        brg_error += 360;
    }

    if (fabsf(dist - dist2) > 1.0f ||
        brg_error > 1.0f) {
        hal.console->printf("Failed offset test brg_error=%f dist_error=%f\n",
                      brg_error, dist-dist2);
    }
}

static const struct {
    float ofs_north, ofs_east, distance, bearing;
} test_offsets[] = {
    { 1000, 1000,  sqrt(2.0f)*1000, 45 },
    { 1000, -1000, sqrt(2.0f)*1000, -45 },
    { 1000, 0,     1000, 0 },
    { 0, 1000,     1000, 90 },
};

static void test_offset(void)
{
    struct Location loc;

    loc.lat = -35*1.0e7f;
    loc.lng = 149*1.0e7f;

    for (uint8_t i=0; i<ARRAY_SIZE(test_offsets); i++) {
        test_one_offset(loc,
                        test_offsets[i].ofs_north,
                        test_offsets[i].ofs_east,
                        test_offsets[i].distance,
                        test_offsets[i].bearing);
    }
}


/*
  test position accuracy for floating point versus integer positions
 */
static void test_accuracy(void)
{
    struct Location loc;

    loc.lat = 0.0e7f;
    loc.lng = -120.0e7f;

    struct Location loc2 = loc;
    Vector2f v((loc.lat*1.0e-7f), (loc.lng*1.0e-7f));
    Vector2f v2;

    loc2 = loc;
    loc2.lat += 10000000;
    v2 = Vector2f(loc2.lat*1.0e-7f, loc2.lng*1.0e-7f);
    hal.console->printf("1 degree lat dist=%.4f\n", get_distance(loc, loc2));

    loc2 = loc;
    loc2.lng += 10000000;
    v2 = Vector2f(loc2.lat*1.0e-7f, loc2.lng*1.0e-7f);
    hal.console->printf("1 degree lng dist=%.4f\n", get_distance(loc, loc2));

    for (int32_t i=0; i<100; i++) {
        loc2 = loc;
        loc2.lat += i;
        v2 = Vector2f((loc.lat+i)*1.0e-7f, loc.lng*1.0e-7f);
        if (v2.x != v.x || v2.y != v.y) {
            hal.console->printf("lat v2 != v at i=%d dist=%.4f\n", (int)i, get_distance(loc, loc2));
            break;
        }
    }
    for (int32_t i=0; i<100; i++) {
        loc2 = loc;
        loc2.lng += i;
        v2 = Vector2f(loc.lat*1.0e-7f, (loc.lng+i)*1.0e-7f);
        if (v2.x != v.x || v2.y != v.y) {
            hal.console->printf("lng v2 != v at i=%d dist=%.4f\n", (int)i, get_distance(loc, loc2));
            break;
        }
    }

    for (int32_t i=0; i<100; i++) {
        loc2 = loc;
        loc2.lat -= i;
        v2 = Vector2f((loc.lat-i)*1.0e-7f, loc.lng*1.0e-7f);
        if (v2.x != v.x || v2.y != v.y) {
            hal.console->printf("-lat v2 != v at i=%d dist=%.4f\n", (int)i, get_distance(loc, loc2));
            break;
        }
    }
    for (int32_t i=0; i<100; i++) {
        loc2 = loc;
        loc2.lng -= i;
        v2 = Vector2f(loc.lat*1.0e-7f, (loc.lng-i)*1.0e-7f);
        if (v2.x != v.x || v2.y != v.y) {
            hal.console->printf("-lng v2 != v at i=%d dist=%.4f\n", (int)i, get_distance(loc, loc2));
            break;
        }
    }
}

static const struct {
    int32_t v, wv;
} wrap_180_tests[] = {
    { 32000,            -4000 },
    { 1500 + 100*36000,  1500 },
    { -1500 - 100*36000, -1500 },
};

static const struct {
    int32_t v, wv;
} wrap_360_tests[] = {
    { 32000,            32000 },
    { 1500 + 100*36000,  1500 },
    { -1500 - 100*36000, 34500 },
};

static const struct {
    float v, wv;
} wrap_PI_tests[] = {
    { 0.2f*PI,            0.2f*PI },
    { 0.2f*PI + 100*PI,  0.2f*PI },
    { -0.2f*PI - 100*PI,  -0.2f*PI },
};

static void test_wrap_cd(void)
{
    for (uint8_t i=0; i < ARRAY_SIZE(wrap_180_tests); i++) {
        int32_t r = wrap_180_cd(wrap_180_tests[i].v);
        if (r != wrap_180_tests[i].wv) {
            hal.console->printf("wrap_180: v=%ld wv=%ld r=%ld\n",
                                (long)wrap_180_tests[i].v,
                                (long)wrap_180_tests[i].wv,
                                (long)r);
        }
    }

    for (uint8_t i=0; i < ARRAY_SIZE(wrap_360_tests); i++) {
        int32_t r = wrap_360_cd(wrap_360_tests[i].v);
        if (r != wrap_360_tests[i].wv) {
            hal.console->printf("wrap_360: v=%ld wv=%ld r=%ld\n",
                                (long)wrap_360_tests[i].v,
                                (long)wrap_360_tests[i].wv,
                                (long)r);
        }
    }

    for (uint8_t i=0; i < ARRAY_SIZE(wrap_PI_tests); i++) {
        float r = wrap_PI(wrap_PI_tests[i].v);
        if (fabsf(r - wrap_PI_tests[i].wv) > 0.001f) {
            hal.console->printf("wrap_PI: v=%f wv=%f r=%f\n",
                                wrap_PI_tests[i].v,
                                wrap_PI_tests[i].wv,
                                r);
        }
    }

    hal.console->printf("wrap_cd tests done\n");
}

#if HAL_CPU_CLASS >= HAL_CPU_CLASS_75
static void test_wgs_conversion_functions(void)
{

    #define D2R DEG_TO_RAD_DOUBLE

    /* Maximum allowable error in quantities with units of length (in meters). */
    #define MAX_DIST_ERROR_M 1e-6
    /* Maximum allowable error in quantities with units of angle (in sec of arc).
     * 1 second of arc on the equator is ~31 meters. */
    #define MAX_ANGLE_ERROR_SEC 1e-7
    #define MAX_ANGLE_ERROR_RAD (MAX_ANGLE_ERROR_SEC*(D2R/3600.0))

    /* Semi-major axis. */
    #define EARTH_A 6378137.0
    /* Semi-minor axis. */
    #define EARTH_B 6356752.31424517929553985595703125


    #define NUM_COORDS 10
    Vector3d llhs[NUM_COORDS];
    llhs[0] = Vector3d(0, 0, 0);        /* On the Equator and Prime Meridian. */
    llhs[1] = Vector3d(0, 180*D2R, 0);  /* On the Equator. */
    llhs[2] = Vector3d(0, 90*D2R, 0);   /* On the Equator. */
    llhs[3] = Vector3d(0, -90*D2R, 0);  /* On the Equator. */
    llhs[4] = Vector3d(90*D2R, 0, 0);   /* North pole. */
    llhs[5] = Vector3d(-90*D2R, 0, 0);  /* South pole. */
    llhs[6] = Vector3d(90*D2R, 0, 22);  /* 22m above the north pole. */
    llhs[7] = Vector3d(-90*D2R, 0, 22); /* 22m above the south pole. */
    llhs[8] = Vector3d(0, 0, 22);       /* 22m above the Equator and Prime Meridian. */
    llhs[9] = Vector3d(0, 180*D2R, 22); /* 22m above the Equator. */

    Vector3d ecefs[NUM_COORDS];
    ecefs[0] = Vector3d(EARTH_A, 0, 0);
    ecefs[1] = Vector3d(-EARTH_A, 0, 0);
    ecefs[2] = Vector3d(0, EARTH_A, 0);
    ecefs[3] = Vector3d(0, -EARTH_A, 0);
    ecefs[4] = Vector3d(0, 0, EARTH_B);
    ecefs[5] = Vector3d(0, 0, -EARTH_B);
    ecefs[6] = Vector3d(0, 0, (EARTH_B+22));
    ecefs[7] = Vector3d(0, 0, -(EARTH_B+22));
    ecefs[8] = Vector3d((22+EARTH_A), 0, 0);
    ecefs[9] = Vector3d(-(22+EARTH_A), 0, 0);

    hal.console->printf("TESTING wgsllh2ecef\n");
    for (int i = 0; i < NUM_COORDS; i++) {

        Vector3d ecef;
        wgsllh2ecef(llhs[i], ecef);

        double x_err = fabs(ecef[0] - ecefs[i][0]);
        double y_err = fabs(ecef[1] - ecefs[i][1]);
        double z_err = fabs(ecef[2] - ecefs[i][2]);
        if ((x_err < MAX_DIST_ERROR_M) &&
                  (y_err < MAX_DIST_ERROR_M) &&
                  (z_err < MAX_DIST_ERROR_M)) {
            hal.console->printf("passing llh to ecef test %d\n", i);
        } else {
            hal.console->printf("failed llh to ecef test %d: ", i);
            hal.console->printf("(%f - %f) (%f - %f) (%f - %f) => %.10f %.10f %.10f\n", ecef[0], ecefs[i][0], ecef[1], ecefs[i][1], ecef[2], ecefs[i][2], x_err, y_err, z_err);
        }

    }

    hal.console->printf("TESTING wgsecef2llh\n");
    for (int i = 0; i < NUM_COORDS; i++) {

        Vector3d llh;
        wgsecef2llh(ecefs[i], llh);

        double lat_err = fabs(llh[0] - llhs[i][0]);
        double lon_err = fabs(llh[1] - llhs[i][1]);
        double hgt_err = fabs(llh[2] - llhs[i][2]);
        if ((lat_err < MAX_ANGLE_ERROR_RAD) &&
                  (lon_err < MAX_ANGLE_ERROR_RAD) &&
                  (hgt_err < MAX_DIST_ERROR_M)) {
            hal.console->printf("passing exef to llh test %d\n", i);
        } else {
            hal.console->printf("failed ecef to llh test %d: ", i);
            hal.console->printf("%.10f %.10f %.10f\n", lat_err, lon_err, hgt_err);

        }

    }
}
#endif //HAL_CPU_CLASS

/*
 *  polygon tests
 */
void setup(void)
{
    test_passed_waypoint();
    test_offset();
    test_accuracy();
    test_wrap_cd();
#if HAL_CPU_CLASS >= HAL_CPU_CLASS_75
    test_wgs_conversion_functions();
#endif
    hal.console->printf("ALL TESTS DONE\n");
}

void loop(void){}

AP_HAL_MAIN();
