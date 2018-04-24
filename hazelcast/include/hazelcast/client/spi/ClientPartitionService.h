/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_CLIENTPARTITIONSERVICE_H_
#define HAZELCAST_CLIENT_SPI_CLIENTPARTITIONSERVICE_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Address;

        namespace impl {
            class Partition;
        }
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace spi {
            /**
             * Partition service for Hazelcast clients.
             *
             * Allows to retrieve information about the partition count, the partition owner or the partitionId of a key.
             */
            class HAZELCAST_API ClientPartitionService {
            public:
                virtual boost::shared_ptr<Address> getPartitionOwner(int partitionId) = 0;

                virtual int getPartitionId(const serialization::pimpl::Data &key) = 0;

                virtual int getPartitionCount() = 0;

                virtual boost::shared_ptr<client::impl::Partition> getPartition(int partitionId) = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_CLIENTPARTITIONSERVICE_H_

