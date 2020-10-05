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

default_logger::default_logger(std::ostream &os, log_level level) 
	: os_(os)
	, level_(level) {}

bool default_logger::enabled(log_level level) noexcept {
	return level >= level_;
}

void default_logger::log(log_level level, const std::string &msg) noexcept {
	auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::lock_guard<std::mutex> g(mut_);
	
	os_ << std::put_time(std::localtime(&t), "%d/%m/%Y %H.%M.%S "); // TODO add millis, cache this

	os_ << level << ": [" << std::this_thread::get_id() << "] " << msg << "\n"; // TODO endl vs \n

	// TODO instance name group name
}

} // namespace hazelcast
