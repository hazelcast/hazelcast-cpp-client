/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <mutex>
#include <deque>
#include <sstream>
#include <fstream>
#include <iterator>

#include <hazelcast/client/hazelcast_client.h>

class rolling_memory_logger
{
public:

    rolling_memory_logger(std::size_t max_content_size)
        :   max_content_size_ {max_content_size}
        ,   content_size_ {}
    {}

    void operator()(const std::string& instance_name,
                    const std::string& cluster_name,
                    hazelcast::logger::level lvl,
                    const std::string& msg)
    {
        std::lock_guard<std::mutex> guard { lock_ };

        std::stringstream ss;

        ss << "{\n"
           << "  \"instance_name\": \"" << instance_name << "\",\n"
           << "  \"cluster_name\": \"" << cluster_name << "\",\n"
           << "  \"level\": \"" << lvl << "\",\n"
           << "  \"message\": \"" << msg << "\"\n"
           << "}\n\n";

        auto new_message = ss.str();

        if (new_message.size() > max_content_size_)
            return;

        while (content_size_ + new_message.size() > max_content_size_)
        {
            auto size = messages_.front().size();
            messages_.pop_front();

            content_size_ -= size;
        }

        content_size_ += new_message.size();
        messages_.push_back(move(new_message));
    }

    void output_to(const std::string& file_name)
    {
        std::ofstream ofs { file_name };

        if (!ofs)
            throw std::runtime_error { "Could not open " + file_name };

        std::lock_guard<std::mutex> guard { lock_ };

        for (const std::string& message : messages_)
            ofs << message;
    }

private:

    std::size_t max_content_size_;
    std::size_t content_size_;
    std::deque<std::string> messages_;
    std::mutex lock_;
};

int
main()
{
    static constexpr std::size_t MAX_LOG_SIZE = 10 * 1024 * 1024; // 10 MB

    rolling_memory_logger logger { MAX_LOG_SIZE };

    hazelcast::client::client_config config;

    // Set custom logger
    config.get_logger_config().handler(std::ref(logger));

    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map("test").get();

    try
    {
        hz.shutdown().get();

        // It will throw exception definitely
        // So the log will be printed to file.
        map->put(5,4).get();
    } catch (const std::exception& e)
    {
        logger.output_to("/tmp/failed.txt");
    }

    return 0;
}
