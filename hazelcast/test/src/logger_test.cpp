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

	std::unique_ptr<logger> lg{ new default_logger(os, log_level::info) };

	ASSERT_FALSE(lg->enabled(log_level::finest));
	ASSERT_TRUE(lg->enabled(log_level::info));
	ASSERT_TRUE(lg->enabled(log_level::warning));
	ASSERT_TRUE(lg->enabled(log_level::severe));
}

TEST(default_logger_test, test_log) {
	std::ostringstream os;

	std::unique_ptr<logger> lg{ new default_logger(os, log_level::info) };

	lg->log(log_level::info, "foo");

	ASSERT_EQ("INFO:foo", os.str());
}

TEST(custom_logger_test, test_enabled_when_not_overridden) {
	struct custom_logger : logger {
		void log(log_level, const std::string&) noexcept override {}
	};

	std::unique_ptr<logger> lg{ new custom_logger };

	ASSERT_TRUE(lg->enabled(log_level::finest));
	ASSERT_TRUE(lg->enabled(log_level::info));
	ASSERT_TRUE(lg->enabled(log_level::warning));
	ASSERT_TRUE(lg->enabled(log_level::severe));
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

	mock_logger lg;
	logger *lg_ptr{ &lg };

	HZ_LOG(lg_ptr, warning, "bar");

	ASSERT_EQ(log_level::warning, lg.level_);
	ASSERT_EQ("bar", lg.msg_);
}

TEST(log_macro_test, test_log_when_disabled) {
	struct mock_logger : logger {
		bool called_{ false };

		void log(log_level level, const std::string &msg) noexcept override {
			called_ = true;
		}

		bool enabled(log_level) noexcept override { return false; }
	};

	mock_logger lg;
	logger *lg_ptr{ &lg };

	HZ_LOG(lg_ptr, warning, "bar");

	ASSERT_FALSE(lg.called_);
}