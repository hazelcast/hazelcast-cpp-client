#include <array>
#include <string>
#include <thread>
#include <sstream>

#include <gtest/gtest.h>

#include "hazelcast/logger.h"

using namespace hazelcast;

TEST(log_level_test, test_output_operator) {
    auto to_str = [](log_level lvl) {
        std::ostringstream os;
        os << lvl;
        return os.str();
    };

    ASSERT_EQ("FINEST", to_str(log_level::finest));
    ASSERT_EQ("INFO", to_str(log_level::info));
    ASSERT_EQ("WARNING", to_str(log_level::warning));
    ASSERT_EQ("SEVERE", to_str(log_level::severe));
    ASSERT_EQ("123", to_str(static_cast<log_level>(123)));
}

TEST(default_logger_test, test_enabled) {
    std::ostringstream os;

    default_logger dlg{ os, log_level::info, "", "" };
    logger &lg = dlg;

    ASSERT_FALSE(lg.enabled(log_level::finest));
    ASSERT_TRUE(lg.enabled(log_level::info));
    ASSERT_TRUE(lg.enabled(log_level::warning));
    ASSERT_TRUE(lg.enabled(log_level::severe));
}

TEST(default_logger_test, test_formatting) {
    std::ostringstream os;

    default_logger dlg{ os, log_level::info, "instance0", "cluster0" };
    logger &lg = dlg;

    lg.log(log_level::info, "message");

    int day, mon, year, hr, mn, sec, ms;
    char lev[64], tid[64], msg[64], ins_grp[64], ver[64];
    int read = std::sscanf(os.str().c_str(), "%02d/%02d/%04d %02d.%02d.%02d.%03d %s %s %s %s %s\n", 
        &day, &mon, &year, &hr, &mn, &sec, &ms, lev, tid, ins_grp, ver, msg);

    ASSERT_EQ(12, read);
    ASSERT_TRUE(0 <= day && day <= 31);
    ASSERT_TRUE(1 <= mon && mon <= 12);
    ASSERT_TRUE(0 <= year && year <= 9999);
    ASSERT_TRUE(0 <= hr && hr <= 23);
    ASSERT_TRUE(0 <= mn && mn <= 59);
    ASSERT_TRUE(0 <= sec && sec <= 59);
    ASSERT_TRUE(0 <= ms && ms <= 999);
    ASSERT_EQ("INFO:", std::string(lev));
    ASSERT_EQ("instance0[cluster0]", std::string(ins_grp));
    ASSERT_EQ(std::string("") + "[" + HAZELCAST_VERSION + "]", std::string(ver));
    ASSERT_EQ("message", std::string(msg));
}

TEST(custom_logger_test, test_enabled_when_not_overridden) {
    struct custom_logger : logger {
        void log(log_level, const std::string&) noexcept override {}
    };

    custom_logger clg;
    logger &lg = clg;

    ASSERT_TRUE(lg.enabled(log_level::finest));
    ASSERT_TRUE(lg.enabled(log_level::info));
    ASSERT_TRUE(lg.enabled(log_level::warning));
    ASSERT_TRUE(lg.enabled(log_level::severe));
}

TEST(log_macro_test, test_log_when_enabled) {
    struct mock_logger : logger {
        log_level level_;
        std::string msg_;

        void log(log_level level, const std::string &msg) noexcept override {
            level_ = level;
            msg_ = msg;
        }

        bool enabled(log_level) noexcept override { return true; }
    };

    mock_logger mlg;
    logger &lg = mlg;

    HZ_LOG(lg, warning, "bar");

    ASSERT_EQ(log_level::warning, mlg.level_);
    ASSERT_EQ("bar", mlg.msg_);
}

TEST(log_macro_test, test_log_when_disabled) {
    struct mock_logger : logger {
        bool called_{ false };

        void log(log_level level, const std::string &msg) noexcept override {
            called_ = true;
        }

        bool enabled(log_level) noexcept override { return false; }
    };

    mock_logger mlg;
    logger &lg = mlg;

    HZ_LOG(lg, warning, "bar");

    ASSERT_FALSE(mlg.called_);
}