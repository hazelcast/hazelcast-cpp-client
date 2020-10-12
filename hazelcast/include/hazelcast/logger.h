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

/**
 * Log severity levels
 */
enum class log_level : int {
    finest = 20,
    info = 50,
    warning = 90,
    severe = 100,
    off = 999
};

HAZELCAST_API std::ostream& operator<<(std::ostream&, log_level level);

/**
 * Base class for all Hazelcast loggers
 * 
 * The user can inherit from this class to customize the logging behavior
 * of the client.
 */
class HAZELCAST_API logger {
public:
    virtual ~logger();

    /**
     * Does the logger want to receive messages of severity level `level`?
     * 
     * Override this method to enable/disable certain severity levels. 
     * The client calls this method to check whether the level is enabled before 
     * calling logger::log.
     *
     * The default implementation always returns `true`.
     *
     * \return true if the logger wants to receive logs of severity level `level`
     */
    virtual bool enabled(log_level level) noexcept;

    /**
     * Handle a log message
     * \see logger::enabled for enabling/disabling certain severity levels.
     * \warning This method must be thread-safe.
     * \param level severity level of the message
     * \param msg contents of the message
     */
    virtual void log(log_level level, const std::string &msg) noexcept = 0;
};

/**
 * The default logger class
 *
 * This class is used as the default hazelcast::logger implementation. It prints each
 * log message as a single line into a given std::ostream. Lines contain the 
 * timestamp, severity level, thread id, instance name, cluster name, library version, 
 * and the message itself.
 */
class HAZELCAST_API default_logger : public logger {
public:
    /**
     * \param os output stream to use
     * \param level minimum enabled log level. messages with 
     * a lower severity level than `level` are ignored.
     * \param instance_name instance name to be written with each message
     * \param cluster_name cluster name to be written with each message
     */
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

/**
 * Create the default logger for a client
 */
HAZELCAST_API std::shared_ptr<logger> make_default_logger(std::string instance_name, std::string cluster_name);

} // namespace hazelcast