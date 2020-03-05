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

#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            IList::IList(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IListImpl(instanceName, context) {
            }

            std::string IList::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler>(
                                getName(), (spi::ClientClusterService &) getContext().getClientClusterService(),
                                getContext().getSerializationService(), listener);
                return proxy::IListImpl::addItemListener(entryEventHandler, includeValue);
            }

            bool IList::removeItemListener(const std::string &registrationId) {
                return proxy::IListImpl::removeItemListener(registrationId);
            }

            int IList::size() {
                return proxy::IListImpl::size();
            }

            bool IList::isEmpty() {
                return size() == 0;
            }

            std::vector<TypedData> IList::toArray() {
                return toTypedDataCollection(proxy::IListImpl::toArrayData());
            }

            void IList::clear() {
                proxy::IListImpl::clear();
            }

            TypedData IList::get(int index) {
                return TypedData(proxy::IListImpl::getData(index), getContext().getSerializationService());
            }

            TypedData IList::remove(int index) {
                return TypedData(proxy::IListImpl::removeData(index), getContext().getSerializationService());
            }

            std::vector<TypedData> IList::subList(int fromIndex, int toIndex) {
                return toTypedDataCollection(proxy::IListImpl::subListData(fromIndex, toIndex));
            }
        }
    }
}
