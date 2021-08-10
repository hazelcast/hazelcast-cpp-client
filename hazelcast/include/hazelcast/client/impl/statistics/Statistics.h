/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <cstdint>
#include <sstream>
#include <memory>

#include <boost/asio/steady_timer.hpp>

#include "hazelcast/client/impl/metrics/metric_descriptor.h"
#include "hazelcast/util/byte.h"
#include "hazelcast/util/export.h"
#include "hazelcast/util/Sync.h"

namespace hazelcast {
    class logger;

    namespace client {
        class address;

        namespace connection {
            class Connection;
        }
        class client_properties;

        namespace spi {
            class ClientContext;
        }
        namespace impl {

            namespace metrics {
                class metrics_compressor;
            }

            namespace statistics {

                class Statistics {
                public:
                    explicit Statistics(spi::ClientContext &client_context);

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
                                          metrics::metrics_compressor &compressor,
                                          const std::shared_ptr<connection::Connection> &connection);

                        void add_near_cache_stats(std::ostringstream &stats, metrics::metrics_compressor &compressor);
                        void add_near_cache_metric(std::ostringstream& stats,
                                                   metrics::metrics_compressor& compressor,
                                                   const std::string &metric,
                                                   const std::string &near_cache_name,
                                                   const std::string &near_cache_name_with_prefix,
                                                   int64_t value,
                                                   metrics::probe_unit unit);

                    private: 
                        template<typename T>
                        void add_stat(std::ostringstream &stats,
                                      const std::string &name,
                                      const T &value)
                        {
                            add_stat(stats, "", name, value);
                        }

                        template<typename T>
                        void add_stat(std::ostringstream &stats, const std::string &key_prefix, const std::string &name,
                                     const T &value) {
                            stats << STAT_SEPARATOR;
                            stats << key_prefix;
                            stats << name << KEY_VALUE_SEPARATOR << value;
                        }

                        /**
                         * @param name the string for which the special characters ',', '=', '\' are escaped properly
                         */
                        void get_name_with_prefix(std::string &name, std::ostringstream &out);

                        Statistics &statistics_;
                    };

                    void schedule_periodic_statistics_send_task(int64_t period_seconds);

                    std::shared_ptr<connection::Connection> get_connection();

                    void send_stats(int64_t timestamp, const std::string &new_stats,
                                   const std::vector<byte> metrics_blob,
                                   const std::shared_ptr<connection::Connection> &connection);

                    static std::string escape_special_characters(std::string &name);

                    spi::ClientContext &client_context_;
                    client_properties &client_properties_;
                    logger &logger_;
                    bool enabled_;
                    PeriodicStatistics periodic_stats_;
                    std::shared_ptr<boost::asio::steady_timer> send_task_timer_;
                };

                template<>
                void Statistics::PeriodicStatistics::add_stat(std::ostringstream &stats, const std::string &name,
                                                             const bool &value);
            }
        }
    }
}



