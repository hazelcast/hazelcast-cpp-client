#pragma once

#include <string>
#include <functional>

#include <hazelcast/util/HazelcastDll.h>

#ifndef HZ_LOGGING_DISABLED
    #define HZ_LOG(lg, lvl, msg) \
        if ((lg).enabled( logger::level::lvl )) { \
            (lg).log(__FILE__, __LINE__, logger::level::lvl, ( msg )); \
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
                                            const char *,
                                            int line,
                                            level,
                                            const std::string &)>;

    logger(std::string instance_name, std::string cluster_name,
           level min_level, handler_type handler);

    bool enabled(level lvl) noexcept;

    void log(const char* file_name, int line, 
             level lvl, const std::string &msg) noexcept;

    static void default_handler(const std::string &instance_name,
                                const std::string &cluster_name,
                                const char *file_name,
                                int line,
                                level lvl,
                                const std::string &msg) noexcept;

private:
    const std::string instance_name_;
    const std::string cluster_name_;
    const level min_level_;
    const handler_type handler_;
};

enum class logger::level : int {
    finest = 20,
    info = 50,
    warning = 90,
    severe = 100,
    off = 999
};

HAZELCAST_API std::ostream& operator<<(std::ostream&, logger::level level);


} // namespace hazelcast