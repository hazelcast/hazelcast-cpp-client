#pragma once

#include <string>
#include <functional>
#include <limits>

#include "hazelcast/util/export.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#ifndef HZ_LOGGING_DISABLED
#define HZ_LOG(lg, lvl, msg) \
        if ((lg).enabled( ::hazelcast::logger::level::lvl )) { \
            (lg).log(::hazelcast::logger::level::lvl, ( msg )); \
        }
#else
    #define HZ_LOG(lg, lvl, msg) 
#endif


namespace hazelcast {

class HAZELCAST_API logger {
public:
    /**
     * Log severity level
     */
    enum class level;

    using handler_type = std::function<void(const std::string &,
                                            const std::string &,
                                            level,
                                            const std::string &)>;

    logger(std::string instance_name, std::string cluster_name,
           level lvl, handler_type handler);

    bool enabled(level lvl) noexcept;

    void log(level lvl, const std::string &msg) noexcept;

    static void default_handler(const std::string &instance_name,
                                const std::string &cluster_name,
                                level lvl,
                                const std::string &msg) noexcept;

private:
    const std::string instance_name_;
    const std::string cluster_name_;
    const level level_;
    const handler_type handler_;
    static std::mutex cout_lock_;
};

enum class logger::level : int {
    all = (std::numeric_limits<int>::min)(),
    finest = 300,
    finer = 400,
    fine = 500,
    info = 800,
    warning = 900,
    severe = 1000,
    off = (std::numeric_limits<int>::max)()
};

HAZELCAST_API std::ostream& operator<<(std::ostream&, logger::level level);

} // namespace hazelcast

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
