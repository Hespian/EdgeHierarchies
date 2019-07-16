#pragma once

#include <iostream>
#include <papi.h>

namespace common {

class papi_result {
    long long counters[3];
    int events[3];

public:
    papi_result() : counters{0, 0, 0}, events{0, 0, 0} {}

    papi_result(int const *e, bool set_to_max = false) {
        long long val = set_to_max ? ((long long)1) << 62 : 0;
        counters[0] = val;
        counters[1] = val;
        counters[2] = val;
        events[0] = e[0];
        events[1] = e[1];
        events[2] = e[2];
    }

    papi_result(int const *e, long long const *c) {
        counters[0] = c[0];
        counters[1] = c[1];
        counters[2] = c[2];
        events[0] = e[0];
        events[1] = e[1];
        events[2] = e[2];
    }

    static std::string describe_event(int event) {
        PAPI_event_info_t info;
        PAPI_get_event_info(event, &info);
        return std::string{(char*)info.short_descr};
    }

    std::ostream &print(std::ostream &os) const {
        return os << describe_event(events[0]) << ": " << counters[0] << "; "
                  << describe_event(events[1]) << ": " << counters[1] << "; "
                  << describe_event(events[2]) << ": " << counters[2] << ".";
    }

    std::ostream &print_component(int component, std::ostream &os) {
        assert(component >= 0 && component <= 2);
        return os << describe_event(events[component]) << ": " << counters[component];
    }

    void compareTo(const papi_result &other) {
        std::cout << describe_event(events[0]) << ": " << (other.counters[0] - counters[0])/(1.0 * other.counters[0]) << "; " << std::endl
                  << describe_event(events[1]) << ": " << (other.counters[1] - counters[1])/(1.0 * other.counters[1]) << "; " << std::endl
                  << describe_event(events[2]) << ": " << (other.counters[2] - counters[2])/(1.0 * other.counters[2]) << "." << std::endl;
    }

    friend std::ostream& operator<<(std::ostream &os, const papi_result &res) {
        return res.print(os);
    }
};

template <int event1 = PAPI_L1_DCM, int event2 = PAPI_L2_DCM, int event3 = PAPI_L3_TCM>
class papi {
    long long counters[3];
    int events[3];
    papi(const papi &) = delete;

public:
    papi(const bool autostart = true) : counters{0, 0, 0}, events{event1, event2, event3} {
        if (PAPI_is_initialized() == PAPI_NOT_INITED &&
            PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
            std::cerr << "PAPI_library_init failed" << std::endl;
        }
        if (autostart) start();
    };

    void start() {
        int ret = PAPI_start_counters(events, 3);
        if (ret != PAPI_OK) {
            std::cerr << "Failed to start PAPI counters" << std::endl;
        }
    }

    void stop() {
        int ret = PAPI_stop_counters(counters, 3);
        if (ret != PAPI_OK) {
            std::cerr << "PAPI_stop_counters failed" << std::endl;
        }
    }

    papi_result result() const {
        return papi_result(events, counters);
    }

    papi_result get_and_reset() {
        stop();
        return result();
    }
};

using papi_cache = papi<PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L3_TCM>;
using papi_instr = papi<PAPI_BR_MSP, PAPI_TOT_INS, PAPI_TOT_CYC>;

}
