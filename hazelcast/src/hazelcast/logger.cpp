#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>

#include "hazelcast/logger.h"

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

void logger::log(const char *file_name, int line, 
                 level lvl, const std::string &msg) noexcept {
    handler_(instance_name_, cluster_name_, file_name, line, lvl, msg);
}

void logger::default_handler(const std::string &instance_name,
                             const std::string &cluster_name, 
                             const char* file_name,
                             int line,
                             level lvl, 
                             const std::string &msg) noexcept {
                             
    auto tp = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(tp);
    
    auto dur = tp.time_since_epoch();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur - sec).count();

    std::ostringstream sstrm;

    sstrm << std::put_time(std::localtime(&t), "%d/%m/%Y %H:%M:%S.")
          << std::setfill('0') << std::setw(3) << ms << ' '
          << lvl << ": [" << std::this_thread::get_id() << "] "
          << instance_name << '[' << cluster_name << "] ["
          << HAZELCAST_VERSION << "] [" // TODO once we have an API for the library version, use that instead
          << file_name << ':' << line << "] "
          << msg
          << '\n';

    {
        static std::mutex cout_lock;
        std::lock_guard<std::mutex> g(cout_lock);
        std::cout << sstrm.str(); // TODO should we flush or not ?
    }
} 


} // namespace hazelcast
