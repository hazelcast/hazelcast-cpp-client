#include <chrono>
#include <string>
#include <sstream>
#include <cstdio>
#include <thread>

#include <gtest/gtest.h>

#include "hazelcast/logger.h"

using hazelcast::logger;

TEST(log_level_test, test_ordering) {
    ASSERT_LT(logger::level::all, logger::level::finest);
    ASSERT_LT(logger::level::finest, logger::level::finer);
    ASSERT_LT(logger::level::finer, logger::level::fine);
    ASSERT_LT(logger::level::fine, logger::level::info);
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
    ASSERT_EQ("FINER", to_str(logger::level::finer));
    ASSERT_EQ("FINE", to_str(logger::level::fine));
    ASSERT_EQ("INFO", to_str(logger::level::info));
    ASSERT_EQ("WARNING", to_str(logger::level::warning));
    ASSERT_EQ("SEVERE", to_str(logger::level::severe));
    ASSERT_EQ("123", to_str(static_cast<logger::level>(123)));
}

TEST(logger_test, test_enabled) {
    std::ostringstream os;

    logger lg{ "", "", logger::level::info, nullptr };

    ASSERT_FALSE(lg.enabled(logger::level::finest));
    ASSERT_FALSE(lg.enabled(logger::level::finer));
    ASSERT_FALSE(lg.enabled(logger::level::fine));
    ASSERT_TRUE(lg.enabled(logger::level::info));
    ASSERT_TRUE(lg.enabled(logger::level::warning));
    ASSERT_TRUE(lg.enabled(logger::level::severe));
}

TEST(logger_test, test_enabled_when_off) {
    std::ostringstream os;

    logger lg{ "", "", logger::level::off, nullptr };

    ASSERT_FALSE(lg.enabled(logger::level::finest));
    ASSERT_FALSE(lg.enabled(logger::level::finer));
    ASSERT_FALSE(lg.enabled(logger::level::fine));
    ASSERT_FALSE(lg.enabled(logger::level::info));
    ASSERT_FALSE(lg.enabled(logger::level::warning));
    ASSERT_FALSE(lg.enabled(logger::level::severe));
}

TEST(logger_test, test_log) {
    struct {
        std::string instance_name;
        std::string cluster_name;
        logger::level level;
        std::string msg;
    } results;

    auto mock_handler = [&results](const std::string &instance_name,
                                   const std::string &cluster_name,
                                   logger::level level, 
                                   const std::string &msg) 
    {
        results.instance_name = instance_name;
        results.cluster_name = cluster_name;
        results.level = level;
        results.msg = msg;
    };

    logger lg{ "instance0", "cluster0", logger::level::info, mock_handler };

    lg.log(logger::level::info, "message");

    ASSERT_EQ("instance0", results.instance_name);
    ASSERT_EQ("cluster0", results.cluster_name);
    ASSERT_EQ(logger::level::info, results.level);
    ASSERT_EQ("message", results.msg);
}

class default_log_handler_test : public ::testing::Test {
protected:
    void SetUp() override {
        old_buffer_ = std::cout.rdbuf();
        std::cout.rdbuf(sstrm_.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(old_buffer_);
    }

    std::streambuf *old_buffer_;
    std::stringstream sstrm_;
};

TEST_F(default_log_handler_test, test_format) {
    logger::default_handler("instance0", "cluster0", logger::level::warning, "message");

    int day, mon, year, hr, mn, sec, ms;
    char lev[64], tid[64], msg[64], ins_grp[64], ver[64];

    int read = std::sscanf(sstrm_.str().c_str(),
                           "%02d/%02d/%04d %02d:%02d:%02d.%03d %s %s %s %s %s\n",
                           &day, &mon, &year, &hr, &mn, &sec, &ms, lev, tid, ins_grp, ver, msg);

    ASSERT_EQ(12, read);

    ASSERT_TRUE(0 <= day && day <= 31);
    ASSERT_TRUE(1 <= mon && mon <= 12);
    ASSERT_TRUE(0 <= year && year <= 9999);
    ASSERT_TRUE(0 <= hr && hr <= 23);
    ASSERT_TRUE(0 <= mn && mn <= 59);
    ASSERT_TRUE(0 <= sec && sec <= 59);
    ASSERT_TRUE(0 <= ms && ms <= 999);
    ASSERT_EQ("WARNING:", std::string(lev));
    std::stringstream expected_tid;
    expected_tid << "[" << std::this_thread::get_id() << "]";
    ASSERT_EQ(expected_tid.str(), std::string(tid));
    ASSERT_EQ("instance0[cluster0]", std::string(ins_grp));
    ASSERT_EQ(std::string() + "[" + HAZELCAST_VERSION + "]", std::string(ver));
    ASSERT_EQ("message", std::string(msg));
}

TEST(log_macro_test, test_log_when_enabled) {
    struct mock_logger {
        bool called{ false };
        logger::level level;
        std::string msg;

        bool enabled(logger::level) {
            return true;
        }
        void log(logger::level l, const std::string &m) {
            called = true;
            this->level = l;
            this->msg = m;
        }
    };

    mock_logger lg;

    HZ_LOG(lg, warning, "message"); 

    ASSERT_TRUE(lg.called);
    ASSERT_EQ(logger::level::warning, lg.level);
    ASSERT_EQ("message", lg.msg);
}

TEST(log_macro_test, test_log_when_disabled) {
    struct mock_logger {
        bool called{ false };

        bool enabled(logger::level level) {
            return false;
        }
        void log(...) {
            called = true;
        }
    };

    mock_logger lg;

    HZ_LOG(lg, warning, "");

    ASSERT_FALSE(lg.called);
}