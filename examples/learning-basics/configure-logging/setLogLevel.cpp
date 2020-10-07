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

    // To modify the default behaviour of loggers,
    // a logger factory must be provided.
    // A logger factory function takes instance and cluster name of the client
    // and return an hazelcast::logger.
    // Here, we instantiate an hazelcast::default_logger with its level
    // set to "finest". 
    config.getLoggerConfig().logger_factory(
        [](std::string instance_name, std::string cluster_name) {
            return std::make_shared<hazelcast::default_logger>(
                std::cout, hazelcast::log_level::finest, instance_name, cluster_name);
        }
    );

    hazelcast::client::HazelcastClient hz(config);

    std::cout << "Finished" << std::endl;

    return 0;
}
