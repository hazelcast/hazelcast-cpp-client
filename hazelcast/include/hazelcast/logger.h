#pragma once

#include <string>
#include <iosfwd>
#include <mutex>

#ifndef HZ_LOGGING_DISABLED
	#define HZ_LOG(logger, level, msg_expr) \
		if ((logger).enabled( log_level::level )) { \
			(logger).log(log_level::level, ( msg_expr )); \
		}
#else
	#define HZ_LOG(logger_ptr, level, msg_expr) 
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
 
class default_logger final : public logger {
public:
	default_logger(std::ostream &os, log_level level);

	bool enabled(log_level level) noexcept override; 
	void log(log_level level, const std::string &msg) noexcept override;

private:
	std::ostream &os_;
	log_level level_;
	std::mutex mut_;
};

} // namespace hazelcast