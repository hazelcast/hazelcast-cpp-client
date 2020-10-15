#include <array>
#include <chrono>
#include <string>
#include <thread>
#include <sstream>

#include <gtest/gtest.h>

#include "hazelcast/logger.h"

using hazelcast::logger;

TEST(log_level_test, test_ordering) {
    ASSERT_LT(logger::level::finest, logger::level::info);
    ASSERT_LT(logger::level::info, logger::level::warning);
    ASSERT_LT(logger::level::warning, logger::level::severe);
    ASSERT_LT(logger::level::severe, logger::level::off);
}

TEST(log_level_test, test_output_operator) {
    auto to_str = [](logger::level lvl) {
        std::ostringstream os;
        os << lvl;
        return os.str();
    };

    ASSERT_EQ("FINEST", to_str(logger::level::finest));
    ASSERT_EQ("INFO", to_str(logger::level::info));
    ASSERT_EQ("WARNING", to_str(logger::level::warning));
    ASSERT_EQ("SEVERE", to_str(logger::level::severe));
    ASSERT_EQ("123", to_str(static_cast<logger::level>(123)));
}

TEST(logger_test, test_enabled) {
    std::ostringstream os;

    logger lg{ "", "", logger::level::info, nullptr };

    ASSERT_FALSE(lg.enabled(logger::level::finest));
    ASSERT_TRUE(lg.enabled(logger::level::info));
    ASSERT_TRUE(lg.enabled(logger::level::warning));
    ASSERT_TRUE(lg.enabled(logger::level::severe));
}


TEST(logger_test, test_log) {
    struct {
        std::string instance_name_;
        std::string cluster_name_;
        std::string file_name_;
        int line_;
        logger::level level_;
        std::string msg_;
    } results;

    auto mock_handler = [&results](const std::string &instance_name,
                                   const std::string &cluster_name, 
                                   const char* file_name,
                                   int line,
                                   logger::level level, 
                                   const std::string &msg) 
    {
        results.instance_name_ = instance_name;
        results.cluster_name_ = cluster_name;
        results.file_name_ = file_name;
        results.line_ = line;
        results.level_ = level;
        results.msg_ = msg;
    };

    logger lg{ "instance0", "cluster0", logger::level::info, mock_handler };

    lg.log("file.cpp", 42, logger::level::info, "message");

    ASSERT_EQ("instance0", results.instance_name_);
    ASSERT_EQ("cluster0", results.cluster_name_);
    ASSERT_EQ("file.cpp", results.file_name_);
    ASSERT_EQ(42, results.line_);
    ASSERT_EQ(logger::level::info, results.level_);
    ASSERT_EQ("message", results.msg_);
}

TEST(log_macro_test, test_log_when_enabled) {
    struct mock_logger {
        bool called_{ false };
        std::string file_name_;
        int line_;
        logger::level level_;
        std::string msg_;

        bool enabled(logger::level level) {
            return true;
        }
        void log(const char* file_name, int line, logger::level level, const std::string &msg) {
            called_ = true;
            file_name_ = file_name;
            line_ = line;
            level_ = level;
            msg_ = msg;
        }
    };

    mock_logger lg;

    #line 42
    HZ_LOG(lg, warning, "message");

    ASSERT_TRUE(lg.called_);
    ASSERT_EQ(__FILE__, lg.file_name_);
    ASSERT_EQ(42, lg.line_);
    ASSERT_EQ(logger::level::warning, lg.level_);
    ASSERT_EQ("message", lg.msg_);
}

TEST(log_macro_test, test_log_when_disabled) {
    struct mock_logger {
        bool called_{ false };

        bool enabled(logger::level level) {
            return false;
        }
        void log(...) {
            called_ = true;
        }
    };

    mock_logger lg;

    HZ_LOG(lg, warning, "");

    ASSERT_FALSE(lg.called_);
}