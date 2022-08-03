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

#include <hazelcast/client/hazelcast_client.h>

void my_log_handler(const std::string &instance_name, 
                    const std::string &cluster_name,
                    hazelcast::logger::level lvl,
                    const std::string &msg) 
{
    // The log handler function can be called concurrently 
    // from different threads. 
    static std::mutex lock;
    std::lock_guard<std::mutex> g(lock);
    
    // Here we print all the log attributes in JSON format 
    // to the standard error.
    std::cerr << "{\n"
              << "  \"instance_name\": \"" << instance_name << "\",\n"
              << "  \"cluster_name\": \"" << cluster_name << "\",\n"
              << "  \"level\": \"" << lvl << "\",\n"
              << "  \"message\": \"" << msg << "\"\n"
              << "}\n\n";
}

int main() {
    hazelcast::client::client_config config;

    // my_log_handler will be called for each log message.
    config.get_logger_config().handler(my_log_handler);

    auto hz = hazelcast::new_client(std::move(config)).get();

    return 0;
}
