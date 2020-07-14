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

#include <boost/optional.hpp>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class Member;

        namespace impl {
            class HAZELCAST_API Partition {
            public:
                /**
                 * Returns the ID of the partition. This value will never change and will always be greater to  or equal to 0 and smaller
                 * than the partition-count.
                 *
                 * @return the ID of the partition
                 */
                virtual int getPartitionId() const = 0;

                /**
                 * Returns the current member that owns this partition.
                 *
                 * The returned value could be stale as soon as it is returned.
                 *
                 * It can be that null is returned if the owner of a partition has not been established.
                 *
                 * @return the owner member of the partition
                 */
                virtual boost::optional<Member> getOwner() const = 0;

                virtual ~Partition() = default;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



