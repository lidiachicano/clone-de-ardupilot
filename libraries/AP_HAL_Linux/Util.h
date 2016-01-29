
#ifndef __AP_HAL_LINUX_UTIL_H__
#define __AP_HAL_LINUX_UTIL_H__

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>

#include "AP_HAL_Linux_Namespace.h"
#include "ToneAlarmDriver.h"
#include "Semaphores.h"

enum hw_type {
    UTIL_HARDWARE_RPI1 = 0,
    UTIL_HARDWARE_RPI2,
    UTIL_HARDWARE_BEBOP,
    UTIL_HARDWARE_BEBOP2,
    UTIL_NUM_HARDWARES,
};

class Linux::Util : public AP_HAL::Util {
public:
    static Util *from(AP_HAL::Util *util) {
        return static_cast<Util*>(util);
    }

    void init(int argc, char * const *argv);
    bool run_debug_shell(AP_HAL::BetterStream *stream) { return false; }

    /**
       return commandline arguments, if available
     */
    void commandline_arguments(uint8_t &argc, char * const *&argv);

    bool toneAlarm_init();
    void toneAlarm_set_tune(uint8_t tune);

    void _toneAlarm_timer_tick();

    /*
      set system clock in UTC microseconds
     */
    void set_system_clock(uint64_t time_utc_usec);    
    const char* get_custom_log_directory() { return custom_log_directory; }
    const char* get_custom_terrain_directory() { return custom_terrain_directory; }

    void set_custom_log_directory(const char *_custom_log_directory) { custom_log_directory = _custom_log_directory; }
    void set_custom_terrain_directory(const char *_custom_terrain_directory) { custom_terrain_directory = _custom_terrain_directory; }

    bool is_chardev_node(const char *path);
    void set_imu_temp(float current);

    uint32_t available_memory(void) override;

    /*
     * Write a string as specified by @fmt to the file in @path. Note this
     * should not be used on hot path since it will open, write and close the
     * file for each call.
     */
    int write_file(const char *path, const char *fmt, ...) FMT_PRINTF(3, 4);

    /*
     * Read a string as specified by @fmt from the file in @path. Note this
     * should not be used on hot path since it will open, read and close the
     * file for each call.
     */
    int read_file(const char *path, const char *fmt, ...) FMT_SCANF(3, 4);

    perf_counter_t perf_alloc(perf_counter_type t, const char *name) override;
    void perf_begin(perf_counter_t perf) override;
    void perf_end(perf_counter_t perf) override;
    void perf_count(perf_counter_t perf) override;

    // create a new semaphore
    AP_HAL::Semaphore *new_semaphore(void) override { return new Linux::Semaphore; }

    int get_hw_arm32();

private:
    static Linux::ToneAlarm _toneAlarm;
    Linux::Heat *_heat;
    int saved_argc;
    char* const *saved_argv;
    const char* custom_log_directory = NULL;
    const char* custom_terrain_directory = NULL;
    static const char *_hw_names[UTIL_NUM_HARDWARES];
};



#endif // __AP_HAL_LINUX_UTIL_H__
