#include <iomanip>
#include <mutex>
#include <ostream>
#include <chrono>
#include <ctime>
#include <thread>

#include "hazelcast/logger.h"

namespace hazelcast {

std::ostream& operator<<(std::ostream &os, log_level level) {
    switch (level) {
        case log_level::severe:
            os << "SEVERE";
            break;
        case log_level::warning:
            os << "WARNING";
            break;
        case log_level::info:
            os << "INFO";
            break;
        case log_level::finest:
            os << "FINEST";
            break;
        default:
            os << static_cast<int>(level);
            break;
    }
    return os;
}

logger::~logger() = default;
bool logger::enabled(log_level level) noexcept { return true; } 

default_logger::default_logger(std::ostream &os, log_level level, std::string instance_name, std::string group_name) 
    : os_(os)
    , level_(level)
    , instance_name_(std::move(instance_name))
    , group_name_(std::move(group_name)) {}

bool default_logger::enabled(log_level level) noexcept {
    return level >= level_;
}

void default_logger::log(log_level level, const std::string &msg) noexcept {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::lock_guard<std::mutex> g(mut_);
    
    os_ << std::put_time(std::localtime(&t), "%d/%m/%Y %H.%M.%S ")  // timestamp  // TODO add milliseconds
        // TODO maybe cache the last result of localtime() and use that directly if we are still in the same second.
        << level << ": [" << std::this_thread::get_id() << "] " // level, thread id
        << instance_name_ << '[' << group_name_ << "] " 
        << "[4.0] " // version  // TODO once we have an API for getting the current version, replace this with that
        << msg // message
        << '\n'; // line break  // TODO should we flush or not ?
}

} // namespace hazelcast
