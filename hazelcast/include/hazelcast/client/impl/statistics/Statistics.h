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

#pragma once

#include <stdint.h>
#include <sstream>
#include <memory>

#include <boost/asio/steady_timer.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Sync.h"

namespace hazelcast {
    class logger;

    namespace client {
        class Address;

        namespace connection {
            class Connection;
        }
        class ClientProperties;

        namespace spi {
            class ClientContext;
        }
        namespace impl {
            namespace statistics {
                class Statistics {
                public:
                    explicit Statistics(spi::ClientContext &clientContext);

                    /**
                     * Registers all client statistics and schedules periodic collection of stats.
                     */
                    void start();

                    void shutdown();

                private:
                    static const std::string NEAR_CACHE_CATEGORY_PREFIX;
                    static const char STAT_SEPARATOR = ',';
                    static const char KEY_VALUE_SEPARATOR = '=';

                    class PeriodicStatistics {
                    public:
                        PeriodicStatistics(Statistics &statistics);

                        void fill_metrics(std::ostringstream &stats,
                                         const std::shared_ptr<connection::Connection> &connection);

                        void add_near_cache_stats(std::ostringstream &stats);

                    private:
                        template<typename T>
                        void add_stat(std::ostringstream &stats, const std::string &name, const T &value) {
                            add_stat(stats, "", name, value);
                        }

                        template<typename T>
                        void add_stat(std::ostringstream &stats, const std::string &keyPrefix, const std::string &name,
                                     const T &value) {
                            stats << STAT_SEPARATOR;
                            stats << keyPrefix;
                            stats << name << KEY_VALUE_SEPARATOR << value;
                        }

                        /**
                         * @param name the string for which the special characters ',', '=', '\' are escaped properly
                         */
                        void get_name_with_prefix(const std::string &name, std::ostringstream &out);

                        Statistics &statistics_;
                    };

                    void schedule_periodic_statistics_send_task(int64_t periodSeconds);

                    std::shared_ptr<connection::Connection> get_connection();

                    void send_stats(int64_t timestamp, const std::string &newStats,
                                   const std::shared_ptr<connection::Connection> &connection);

                    static std::string escape_special_characters(const std::string &name);

                    spi::ClientContext &clientContext_;
                    ClientProperties &clientProperties_;
                    logger &logger_;
                    bool enabled_;
                    PeriodicStatistics periodicStats_;
                    util::Sync<std::shared_ptr<Address> > cachedOwnerAddress_;
                    std::shared_ptr<boost::asio::steady_timer> sendTaskTimer_;
                };

                template<>
                void Statistics::PeriodicStatistics::add_stat(std::ostringstream &stats, const std::string &name,
                                                             const bool &value);
            }
        }
    }
}



