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

    default_logger dlg{ os, log_level::info, "instance0", "group0" };
    logger &lg = dlg;

    lg.log(log_level::info, "message");

    int day, mon, year, hr, mn, sec;
    char lev[16], tid[16], msg[16], ins_grp[32], ver[16];
    int read = std::sscanf(os.str().c_str(), "%02d/%02d/%04d %02d.%02d.%02d %s %s %s %s %s\n", 
        &day, &mon, &year, &hr, &mn, &sec, lev, tid, ins_grp, ver, msg);

    ASSERT_EQ(11, read);
    ASSERT_TRUE(0 <= day && day <= 31);
    ASSERT_TRUE(1 <= mon && mon <= 12);
    ASSERT_TRUE(0 <= year && year <= 9999);
    ASSERT_EQ("INFO:", std::string(lev));
    ASSERT_EQ("instance0[group0]", std::string(ins_grp));
    ASSERT_EQ("[4.0]", std::string(ver));
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