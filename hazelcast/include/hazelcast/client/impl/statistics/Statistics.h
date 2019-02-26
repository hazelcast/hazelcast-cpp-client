/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_IMPL_STATISTICS_STATISTICS_H_
#define HAZELCAST_CLIENT_IMPL_STATISTICS_STATISTICS_H_

#include <stdint.h>
#include <sstream>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/Atomic.h"

namespace hazelcast {
    namespace util {
        class ILogger;
    }
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

                private:
                    static const std::string NEAR_CACHE_CATEGORY_PREFIX;
                    static const std::string FEATURE_SUPPORTED_SINCE_VERSION_STRING;
                    static const int FEATURE_SUPPORTED_SINCE_VERSION;
                    static const char STAT_SEPARATOR = ',';
                    static const char KEY_VALUE_SEPARATOR = '=';
                    static const int MILLIS_IN_A_SECOND = 1000;

                    class PeriodicStatistics {
                    public:
                        PeriodicStatistics(Statistics &statistics);

                        void fillMetrics(std::ostringstream &stats,
                                         const boost::shared_ptr<connection::Connection> &ownerConnection);

                        void addNearCacheStats(std::ostringstream &stats);

                    private:
                        template<typename T>
                        void addStat(std::ostringstream &stats, const std::string &name, const T &value) {
                            addStat(stats, "", name, value);
                        }

                        template<typename T>
                        void addStat(std::ostringstream &stats, const std::string &keyPrefix, const std::string &name,
                                     const T &value) {
                            stats << STAT_SEPARATOR;
                            stats << keyPrefix;
                            stats << name << KEY_VALUE_SEPARATOR << value;
                        }

                        /**
                         * @param name the string for which the special characters ',', '=', '\' are escaped properly
                         */
                        void getNameWithPrefix(const std::string &name, std::ostringstream &out);

                        Statistics &statistics;
                    };

                    class CollectStatisticsTask : public util::Runnable {
                    public:
                        CollectStatisticsTask(Statistics &statistics);

                        virtual const std::string getName() const;

                        virtual void run();

                    private:
                        Statistics &statistics;
                    };

                    void schedulePeriodicStatisticsSendTask(int64_t periodSeconds);

                    boost::shared_ptr<connection::Connection> getOwnerConnection();

                    void sendStats(const std::string &newStats,
                                   const boost::shared_ptr<connection::Connection> &ownerConnection);

                    bool isSameWithCachedOwnerAddress(const boost::shared_ptr<Address> &currentOwnerAddress);

                    static std::string escapeSpecialCharacters(const std::string &name);

                    spi::ClientContext &clientContext;
                    ClientProperties &clientProperties;
                    util::ILogger &logger;
                    bool enabled;
                    PeriodicStatistics periodicStats;
                    util::Atomic<boost::shared_ptr<Address> > cachedOwnerAddress;
                };

                template<>
                void Statistics::PeriodicStatistics::addStat(std::ostringstream &stats, const std::string &name,
                                                             const bool &value);
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_STATISTICS_STATISTICS_H_

