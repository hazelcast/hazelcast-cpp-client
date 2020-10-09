#pragma once

#include <string>
#include <iosfwd>
#include <mutex>
#include <memory>

#include <hazelcast/util/HazelcastDll.h>

#ifndef HZ_LOGGING_DISABLED
    #define HZ_LOG(logger, level, msg) \
        if ((logger).enabled( log_level::level )) { \
            (logger).log(log_level::level, ( msg )); \
        }
#else
    #define HZ_LOG(logger, level, msg) 
#endif

namespace hazelcast {

enum class log_level : int {
    finest = 20,
    info = 50,
    warning = 90,
    severe = 100
};

std::ostream& HAZELCAST_API operator<<(std::ostream&, log_level level);

class HAZELCAST_API logger {
public:
    virtual ~logger();

    virtual bool enabled(log_level level) noexcept;
    virtual void log(log_level level, const std::string &msg) noexcept = 0;
};
 
class HAZELCAST_API default_logger : public logger {
public:
    default_logger(std::ostream &os, log_level level, 
        std::string instance_name, std::string cluster_name);

    bool enabled(log_level level) noexcept override; 
    void log(log_level level, const std::string &msg) noexcept override;

private:
    std::ostream &os_;
    log_level level_;
    std::mutex mut_;
    std::string instance_name_;
    std::string cluster_name_;
};

std::shared_ptr<logger> HAZELCAST_API make_default_logger(std::string instance_name, std::string cluster_name);

} // namespace hazelcast