#pragma once

#include <string>
#include <iosfwd>
#include <mutex>

// TODO the `logger` interface and the implementation `default_logger` can be declared in different header files.

#ifndef HZ_LOGGING_DISABLED
	#define HZ_LOG(logger, level, msg) \
		if ((logger).enabled( log_level::level )) { \
			(logger).log(log_level::level, ( msg )); \
		}
#else
	#define HZ_LOG(logger_ptr, level, msg) 
#endif

namespace hazelcast {

enum class log_level : int {
	finest = 20,
	info = 50,
	warning = 90,
	severe = 100
};

std::ostream& operator<<(std::ostream&, log_level level);

class logger {
public:
	virtual ~logger();

	virtual bool enabled(log_level level) noexcept;
	virtual void log(log_level level, const std::string &msg) noexcept = 0;
};
 
class default_logger : public logger {
public:
	default_logger(std::ostream &os, log_level level, 
		std::string instance_name, std::string group_name);

	bool enabled(log_level level) noexcept override; 
	void log(log_level level, const std::string &msg) noexcept override;

private:
	std::ostream &os_;
	log_level level_;
	std::mutex mut_;
	std::string instance_name_;
	std::string group_name_;
};

} // namespace hazelcast