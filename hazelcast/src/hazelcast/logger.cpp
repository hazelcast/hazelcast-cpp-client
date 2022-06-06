#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>

#include "hazelcast/logger.h"
#include "hazelcast/client/hazelcast_client.h"

namespace hazelcast {

std::ostream& operator<<(std::ostream &os, logger::level lvl) {
    switch (lvl) {
        case logger::level::severe:
            os << "SEVERE";
            break;
        case logger::level::warning:
            os << "WARNING";
            break;
        case logger::level::info:
            os << "INFO";
            break;
        case logger::level::fine:
            os << "FINE";
            break;
        case logger::level::finer:
            os << "FINER";
            break;
        case logger::level::finest:
            os << "FINEST";
            break;
        default:
            os << static_cast<int>(lvl);
            break;
    }
    return os;
}

logger::logger(std::string instance_name, std::string cluster_name, level level, handler_type handler)
    : instance_name_{ std::move(instance_name) }
    , cluster_name_{ std::move(cluster_name) }
    , level_{ level }
    , handler_{ std::move(handler) } 
{}

bool logger::enabled(level lvl) noexcept {
    return lvl >= level_;
}

void logger::log(level lvl, const std::string &msg) noexcept {
    handler_(instance_name_, cluster_name_, lvl, msg);
}

namespace {

std::tm time_t_to_localtime(const std::time_t &t) {
    std::tm lt;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    ::localtime_s(&lt, &t);
#else
    ::localtime_r(&t, &lt);
#endif

    return lt;
}

}

std::mutex logger::cout_lock_;

void logger::default_handler(const std::string &instance_name,
                             const std::string &cluster_name,
                             level lvl, 
                             const std::string &msg) noexcept {

    auto tp = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(tp);
    auto local_t = time_t_to_localtime(t);

    auto dur = tp.time_since_epoch();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur - sec).count();

    std::ostringstream sstrm;

    char time_buffer[80];
    std::strftime(time_buffer, sizeof(time_buffer), "%d/%m/%Y %H:%M:%S.", &local_t);
    sstrm << time_buffer
          << std::setfill('0') << std::setw(3) << ms << ' '
          << lvl << ": [" << std::this_thread::get_id() << "] "
          << instance_name << '[' << cluster_name << "] ["
          << client::version() << "] "
          << msg
          << '\n';

    {
        std::lock_guard<std::mutex> g(cout_lock_);
        std::cout << sstrm.str() << std::flush;
    }
} 


} // namespace hazelcast
