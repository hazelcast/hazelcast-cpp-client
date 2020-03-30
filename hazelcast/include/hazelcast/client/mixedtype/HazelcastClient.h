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
#ifndef HAZELCAST_CLIENT_MIXEDTYPE_HAZELCASTCLIENT_H_
#define HAZELCAST_CLIENT_MIXEDTYPE_HAZELCASTCLIENT_H_

#include <vector>
#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/mixedtype/IMap.h"
#include "hazelcast/client/mixedtype/MultiMap.h"
#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/client/mixedtype/ISet.h"
#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/mixedtype/ITopic.h"
#include "hazelcast/client/mixedtype/Ringbuffer.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            class HAZELCAST_API HazelcastClient {
            public:
                virtual ~HazelcastClient() {}

                /**
                *
                * Returns the distributed map instance with the specified name.
                *
                * @param name name of the distributed map
                * @return distributed map instance with the specified name
                */
                virtual IMap getMap(const std::string &name) = 0;

                /**
                * Returns the distributed multimap instance with the specified name.
                *
                * @param name name of the distributed multimap
                * @return distributed multimap instance with the specified name
                */
                virtual MultiMap getMultiMap(const std::string &name) = 0;

                /**
                * Returns the distributed queue instance with the specified name.
                *
                * @param name name of the distributed queue
                * @return distributed queue instance with the specified name
                */
                virtual IQueue getQueue(const std::string &name) = 0;

                /**
                * Returns the distributed set instance with the specified name.
                * Set is ordered unique set of entries. similar to std::set
                *
                * @param name name of the distributed set
                * @return distributed set instance with the specified name
                */
                virtual ISet getSet(const std::string &name) = 0;

                /**
                * Returns the distributed list instance with the specified name.
                * List is ordered set of entries. similar to std::vector
                *
                * @param name name of the distributed list
                * @return distributed list instance with the specified name
                */
                virtual IList getList(const std::string &name) = 0;

                /**
                * Returns the distributed topic instance with the specified name.
                *
                * @param name name of the distributed topic
                * @return distributed topic instance with the specified name
                */
                virtual ITopic getTopic(const std::string& name) = 0;

                /**
                 * Returns the distributed Ringbuffer instance with the specified name.
                 *
                 * @param instanceName name of the distributed Ringbuffer
                 * @return distributed RingBuffer instance with the specified name
                 */
                virtual Ringbuffer getRingbuffer(const std::string &instanceName) = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MIXEDTYPE_HAZELCASTCLIENT_H_ */

