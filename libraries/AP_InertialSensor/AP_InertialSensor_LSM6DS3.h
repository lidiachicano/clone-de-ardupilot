#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/I2CDevice.h>

#include "stdio.h"
#include "AP_InertialSensor.h"
#include "AP_InertialSensor_Backend.h"
#define HAL_INS_LSM6DS3_I2C_BUS 	0
#define HAL_INS_LSM6DS3_I2C_ADDR	0x6B

class AP_InertialSensor_LSM6DS3 : public AP_InertialSensor_Backend
{
public:
    virtual ~AP_InertialSensor_LSM6DS3() { }
    void start(void) override;
    bool update() override;

    static AP_InertialSensor_Backend *probe(AP_InertialSensor &imu,AP_HAL::OwnPtr<AP_HAL::I2CDevice> dev);

private:
    AP_InertialSensor_LSM6DS3(AP_InertialSensor &imu,AP_HAL::OwnPtr<AP_HAL::I2CDevice> dev);

    struct PACKED sensor_raw_data {
        int16_t x;
        int16_t y;
        int16_t z;
    };

    enum gyro_scale {
		G_SCALE_125DPS = 0,
        G_SCALE_245DPS ,
        G_SCALE_500DPS,
		G_SCALE_1000DPS,
        G_SCALE_2000DPS,
    };

    enum accel_scale {
        A_SCALE_2G = 2,
        A_SCALE_4G = 4,
        A_SCALE_8G = 8,
        A_SCALE_16G = 16,
    };

    bool _accel_data_ready(uint8_t status);
    bool _gyro_data_ready(uint8_t status);
    void _poll_data();
    bool _init();
    void _gyro_init();
    void _accel_init();
    void _set_gyro_scale(gyro_scale scale);
    void _set_accel_scale(accel_scale scale);
    uint8_t _register_read(uint8_t reg);
    void _register_write(uint8_t reg, uint8_t val, bool checked=false);
    void _read_data_transaction_a();
    void _read_data_transaction_g();

AP_HAL::OwnPtr<AP_HAL::I2CDevice> _dev;

    /*
     * If data-ready GPIO pins numbers are not defined (i.e. any negative
     * value), the fallback approach used is to check if there's new data ready
     * by reading the status register. It is *strongly* recommended to use
     * data-ready GPIO pins for performance reasons.
     */
    float _gyro_scale;
    float _accel_scale;
    uint8_t _gyro_instance;
    uint8_t _accel_instance;

    // gyro whoami
    uint8_t whoami_g;

};
