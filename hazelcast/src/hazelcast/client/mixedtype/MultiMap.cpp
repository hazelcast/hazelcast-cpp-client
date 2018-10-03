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

#include "hazelcast/client/mixedtype/MultiMap.h"
#include "hazelcast/client/impl/ItemEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            MultiMap::MultiMap(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::MultiMapImpl(instanceName, context) {
            }

            std::vector<TypedData> MultiMap::keySet() {
                return toTypedDataCollection(proxy::MultiMapImpl::keySetData());
            }

            std::vector<TypedData> MultiMap::values() {
                return toTypedDataCollection(proxy::MultiMapImpl::valuesData());
            }

            std::vector<std::pair<TypedData, TypedData> > MultiMap::entrySet() {
                return toTypedDataEntrySet(proxy::MultiMapImpl::entrySetData());
            }

            int MultiMap::size() {
                return proxy::MultiMapImpl::size();
            }

            void MultiMap::clear() {
                proxy::MultiMapImpl::clear();
            }

            std::string MultiMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                spi::ClientClusterService &clusterService = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), clusterService, ss, listener, includeValue);
                return proxy::MultiMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            bool MultiMap::removeEntryListener(const std::string &registrationId) {
                return proxy::MultiMapImpl::removeEntryListener(registrationId);
            }

        }
    }
}
