#include <stdio.h>
#include <sys/time.h>
#include <webots/supervisor.h>
#include "util_time.h"
#include "sensors.h"

#define M_PI  3.14159265358979323846
#define M_PI2 6.28318530718


/*
https://discuss.ardupilot.org/t/copter-x-y-z-which-is-which/6823/2

Copy pasted what’s important:
NED Coordinate System:

The x axis is aligned with the vector to the north pole (tangent to meridians).
The y axis points to the east side (tangent to parallels)
The z axis points to the center of the earth
There is also Body Fixed Frame:
Body Fixed Frame (Attached to the aircraft)

The x axis points in forward (defined by geometry and not by movement) direction. (= roll axis)
The y axis points to the right (geometrically) (= pitch axis)
The z axis points downwards (geometrically) (= yaw axis)
In order to convert from Body Frame to NED what you need to call this function:

copter.rotate_body_frame_to_NE(vel_vector.x, vel_vector.y);




 */
/*
  returns: "yaw":_6.594794831471518e-05,"pitch":_-0.0005172680830582976,"roll":_0.022908752784132957}}
*/
void getInertia (const WbDeviceTag inertialUnit, char *buf)
{
  const double *inertial_directions = wb_inertial_unit_get_roll_pitch_yaw (inertialUnit);
  sprintf(buf,"\"roll\": %f,\"pitch\": %f,\"yaw\": %f",inertial_directions[0], inertial_directions[1], inertial_directions[2]);

    return ;
}

/*
  returns: "magnetic_field":_[23088.669921875,_3876.001220703125,_-53204.57421875]
*/
void getCompass (const WbDeviceTag compass, char *buf)
{
    const double *north3D = wb_compass_get_values(compass);
    sprintf(buf,"[%f, %f, %f]",north3D[0], north3D[2], north3D[1]);
    //sprintf(buf,"[0.0, 0.0, 0.0]");

    return ;
}



/*
  returns: "vehicle.gps":{"timestamp":_1563301031.055164,"x":_5.5127296946011484e-05,"y":_-0.0010968948481604457,"z":_0.037179552018642426}, 
*/
void getGPS (const WbDeviceTag gps, char *buf)
{

    const double *north3D = wb_gps_get_values(gps);
    sprintf(buf,"\"x\": %f,\"y\": %f,\"z\": %f", north3D[0], north3D[2], north3D[1]);

    return ;
}

/*
 returns: "linear_acceleration": [0.005074390675872564, 0.22471477091312408, 9.80740737915039]
*/
void getAcc (const WbDeviceTag accelerometer, char *buf)
{
    //SHOULD BE CORRECT 
    const double *a = wb_accelerometer_get_values(accelerometer);
    sprintf(buf,"[%f, %f, %f]",a[0], a[2], a[1]);
    //sprintf(buf,"[0.0, 0.0, 0.0]");

    return ;
}


/*
  returns: "angular_velocity": [-1.0255117643964695e-07, -8.877226775894087e-08, 2.087078510015772e-09]
*/
void getGyro (const WbDeviceTag gyro, char *buf)
{

    const double *g = wb_gyro_get_values(gyro);
    sprintf(buf,"[%f, %f, %f]",g[0], g[2], g[1]);
    //sprintf(buf,"[0.0, 0.0, 0.0]");

    return ;
}


void getLinearVelocity (WbNodeRef nodeRef, char * buf)
{
    const double * vel = wb_supervisor_node_get_velocity (nodeRef);
    if (vel != NULL)
    {
      sprintf (buf,"[%f, %f, %f]", vel[0],vel[2],vel[1]); 
    }
    
}

void getAllSensors (char *buf, WbDeviceTag gyro, WbDeviceTag accelerometer, WbDeviceTag compass, const WbDeviceTag gps, WbDeviceTag inertial_unit)
{

/*

{"timestamp": 1564002470.221124007,
      "vehicle.imu": {
            "timestamp": 1564002470.221124007,
            "angular_velocity": [0.000000, 0.000000, -0.000000],
            "linear_acceleration": [0.001169, -0.001169, 9.806650],
            "magnetic_field": [0.991097, 0.133143, -0.000102],
            "vehicle.gps":{
                "timestamp": 1564002470.221124007,
                "x": -0.123689,
                "y": 0.073021,
                "z": 0.049984},
            "vehicle.velocity":{
                "timestamp": 1564002470.221124007,
                "linear_velocity": [0.000000, 0.000000, -0.000000],
                "angular_velocity":[-0.000000, 0.000000, -0.000000], 
                "world_linear_velocity": [ 0.0, 0.0, 0.0]},
            "vehicle.pose":{
                "timestamp": 1564002470.221124007,
                "x": -0.123689,
                "y": 0.073021,
                "z": 0.049984,
                "roll": 0.000119,
                "pitch": 0.000119,
                "yaw": 0.133539}
                }



{"timestamp": 1563544049.2840538, 
    "vehicle.imu": {"timestamp": 1563544049.2673872, 
    "angular_velocity": [-2.0466000023589004e-06, 3.1428675129063777e-07, -6.141597896913709e-09],
    "linear_acceleration": [0.005077465437352657, 0.22471386194229126, 9.807389259338379], 
    "magnetic_field": [23088.71484375, 3875.498046875, -53204.48046875]}, 
    "vehicle.gps": {
        "timestamp": 1563544049.2673872, 
        "x": 4.985610576113686e-05, "y": -0.0010973707539960742, "z": 0.037179529666900635}, 
    "vehicle.velocity": {"timestamp": 1563544049.2673872, 
                        "linear_velocity": [-3.12359499377024e-10, -1.3824124955874595e-08, -6.033386625858839e-07],
                        "angular_velocity": [-2.0466000023589004e-06, 3.1428675129063777e-07, -6.141597896913709e-09], 
                        "world_linear_velocity": [0.0, 0.0, -6.034970851942489e-07]}, 
    "vehicle.pose": {"timestamp": 1563544049.2673872, 
                            "x": 4.985610576113686e-05, "y": -0.0010973707539960742, "z": 0.037179529666900635, "yaw": 8.899446402210742e-05, "pitch": -0.0005175824626348913, "roll": 0.022908702492713928}
                            }
*/


        static char compass_buf [150];
        static char acc_buf [150];
        static char gyro_buf [150];
        static char gps_buf [150];
        static char inertial_buf [150];
        static char linear_velocity_buf [150];

        char szTime[21];
        struct timespec ts = getMilliseconds();
        double time = wb_robot_get_time(); // current simulation time in [s]
        //printf ("time %lld.%.9ld %d\n", (long long)ts.tv_sec, ts.tv_nsec,time);
        sprintf(szTime,"%lld.%.9ld", (long long)ts.tv_sec, ts.tv_nsec);
        
        getGyro(gyro, gyro_buf);
        getAcc(accelerometer, acc_buf);
        getCompass(compass, compass_buf);
        getGPS(gps, gps_buf);
        getInertia (inertial_unit, inertial_buf);
        getLinearVelocity(wb_supervisor_node_get_self(), linear_velocity_buf);

        sprintf (buf,"{\"timestamp\": %s,\"vehicle.imu\": {\"timestamp\": %s,\"angular_velocity\": %s,\"linear_acceleration\": %s,\"magnetic_field\": %s,\"vehicle.gps\":{\"timestamp\": %s,%s},\"vehicle.velocity\":{\"timestamp\": %s,\"world_linear_velocity\": %s},\"vehicle.pose\":{\"timestamp\": %s,%s,%s}}\r\n"
                                  , szTime,                             szTime,                 gyro_buf,                    acc_buf,                 compass_buf,               szTime, gps_buf,                                  szTime,                 linear_velocity_buf,               szTime, gps_buf, inertial_buf );

}