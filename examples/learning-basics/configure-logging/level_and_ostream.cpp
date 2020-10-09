/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <memory>

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/logger.h>

int main() {
    hazelcast::client::ClientConfig config;

    // By default, the client prints log messages of severity log_level::info or above
    // to standard output.
    // To modify the default behaviour, a logger factory must be set in the configuration
    // object. The logger factory function takes instance and cluster name of the client
    // and return an hazelcast::logger.
    // Here, we return an hazelcast::default_logger with level log_level::finest that uses
    // std::cerr to print log messages.
    config.getLoggerConfig().logger_factory(
        [](std::string instance_name, std::string cluster_name) {
            return std::make_shared<hazelcast::default_logger>(
                std::cerr, hazelcast::log_level::finest, instance_name, cluster_name);
        }
    );

    hazelcast::client::HazelcastClient hz(config);

    std::cout << "Finished" << std::endl;

    return 0;
}
