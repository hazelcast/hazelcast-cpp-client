#include <chrono>
#include <string>
#include <sstream>
#include <cstdio>
#include <thread>

#include <gtest/gtest.h>

#include "hazelcast/logger.h"

using hazelcast::logger;

TEST(log_level_test, test_ordering) {
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

class default_log_handler_test : public ::testing::Test {
protected:
    void SetUp() override {
        old_buffer = std::cout.rdbuf();
        std::cout.rdbuf(sstrm.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(old_buffer);
    }

    std::streambuf *old_buffer;
    std::stringstream sstrm;
};

TEST_F(default_log_handler_test, test_format) {
    logger::default_handler("instance0", "cluster0", "file.cpp", 123, 
                            logger::level::warning, "message");

    int day, mon, year, hr, mn, sec, ms;
    char lev[64], tid[64], msg[64], ins_grp[64], ver[64], file_line[256];

    int read = std::sscanf(sstrm.str().c_str(), 
        "%02d/%02d/%04d %02d:%02d:%02d.%03d %s %s %s %s %s %s\n", 
         &day, &mon, &year, &hr, &mn, &sec, &ms, lev, tid, ins_grp, ver, file_line, msg);

    ASSERT_EQ(13, read);

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
    ASSERT_EQ("[file.cpp:123]", std::string(file_line));
    ASSERT_EQ("message", std::string(msg));
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

    HZ_LOG(lg, warning, "message"); 
    int expected_line = __LINE__ - 1;

    ASSERT_TRUE(lg.called_);
    ASSERT_EQ(__FILE__, lg.file_name_);
    ASSERT_EQ(expected_line, lg.line_);
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