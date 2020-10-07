#include <iomanip>
#include <iostream>
#include <mutex>
#include <ostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>

#include "hazelcast/logger.h"

namespace hazelcast {

namespace {

std::ostream &timestamp(std::ostream &os) {
    auto tp = std::chrono::system_clock::now();

    auto t = std::chrono::system_clock::to_time_t(tp);
    os << std::put_time(std::localtime(&t), "%d/%m/%Y %H.%M.%S.");

    auto dur = tp.time_since_epoch();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur - sec).count();

    auto old_f = os.fill('0');
    auto old_w = os.width(3);
    os << ms;
    os.fill(old_f);
    os.width(old_w);

    return os;
}

}

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

default_logger::default_logger(std::ostream &os, log_level level, std::string instance_name, std::string cluster_name) 
    : os_(os)
    , level_(level)
    , instance_name_(std::move(instance_name))
    , cluster_name_(std::move(cluster_name)) {}

bool default_logger::enabled(log_level level) noexcept {
    return level >= level_;
}

void default_logger::log(log_level level, const std::string &msg) noexcept {
    std::ostringstream sstrm;

    sstrm << timestamp << ' ' // timestamp
          << level << ": [" << std::this_thread::get_id() << "] " // level, thread id
          << instance_name_ << '[' << cluster_name_ << "] ["
          << HAZELCAST_VERSION << "] " // version  // TODO once we have an API for getting the current version, replace this with that
          << msg // message
          << '\n'; // line break

    {
        std::lock_guard<std::mutex> g(mut_);
        os_ << sstrm.str(); // TODO should we flush or not ?
    }
}

std::shared_ptr<logger> make_default_logger(std::string instance_name, std::string cluster_name) {
    return std::make_shared<default_logger>(std::cout, log_level::info, 
                                            std::move(instance_name), std::move(cluster_name));
}

} // namespace hazelcast
