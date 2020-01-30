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

#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"
#include "hazelcast/client/map/impl/MapMixedTypeProxyFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            namespace impl {
                HazelcastClientImpl::HazelcastClientImpl(client::impl::HazelcastClientInstanceImpl &client) : client(client) {
                }

                HazelcastClientImpl::~HazelcastClientImpl() {
                }

                IMap HazelcastClientImpl::getMap(const std::string &name) {
                    map::impl::MapMixedTypeProxyFactory factory(&client.clientContext);
                    std::shared_ptr<spi::ClientProxy> proxy =
                            client.getDistributedObjectForService("hz:impl:mapService", name, factory);
                    return IMap(std::static_pointer_cast<ClientMapProxy>(proxy));
                }

                MultiMap HazelcastClientImpl::getMultiMap(const std::string &name) {
                    return client.getDistributedObject<MultiMap>(name);
                }

                IQueue HazelcastClientImpl::getQueue(const std::string &name) {
                    return client.getDistributedObject<IQueue>(name);
                }

                ISet HazelcastClientImpl::getSet(const std::string &name) {
                    return client.getDistributedObject<ISet>(name);
                }

                IList HazelcastClientImpl::getList(const std::string &name) {
                    return client.getDistributedObject<IList>(name);
                }

                ITopic HazelcastClientImpl::getTopic(const std::string &name) {
                    return client.getDistributedObject<ITopic>(name);
                }

                Ringbuffer HazelcastClientImpl::getRingbuffer(
                        const std::string &instanceName) {
                    return client.getDistributedObject<Ringbuffer>(instanceName);
                }

            }
        }
    }
}
