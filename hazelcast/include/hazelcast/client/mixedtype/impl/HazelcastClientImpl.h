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
#ifndef HAZELCAST_CLIENT_MIXEDTYPE_IMPL_HAZELCASTCLIENTIMPL_H_
#define HAZELCAST_CLIENT_MIXEDTYPE_IMPL_HAZELCASTCLIENTIMPL_H_

#include <vector>
#include <string>

#include "hazelcast/client/mixedtype/HazelcastClient.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class HazelcastClientInstanceImpl;
        }
        namespace spi {
            class ClientContext;
        }

        namespace mixedtype {
            namespace impl {
                class HAZELCAST_API HazelcastClientImpl : public HazelcastClient {
                public:
                    HazelcastClientImpl(client::impl::HazelcastClientInstanceImpl &client);

                    virtual ~HazelcastClientImpl();

                    virtual IMap getMap(const std::string &name);

                    virtual MultiMap getMultiMap(const std::string &name);

                    virtual IQueue getQueue(const std::string &name);

                    virtual ISet getSet(const std::string &name);

                    virtual IList getList(const std::string &name);

                    virtual ITopic getTopic(const std::string &name);

                    virtual Ringbuffer getRingbuffer(const std::string &instanceName);
                private:
                    client::impl::HazelcastClientInstanceImpl &client;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MIXEDTYPE_IMPL_HAZELCASTCLIENTIMPL_H_ */

