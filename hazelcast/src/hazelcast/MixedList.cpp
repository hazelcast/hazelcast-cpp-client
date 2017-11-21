/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/MixedList.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/ListAddListenerCodec.h"

namespace hazelcast {
    namespace client {
        MixedList::MixedList(const std::string &instanceName, spi::ClientContext *context)
                : proxy::IListImpl(instanceName, context) {
        }

        std::string MixedList::addItemListener(MixedItemListener &listener, bool includeValue) {
            impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler> *entryEventHandler =
                    new impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler>(
                            getName(), context->getClusterService(), context->getSerializationService(), listener);
            return proxy::IListImpl::addItemListener(entryEventHandler, includeValue);
        }

        bool MixedList::removeItemListener(const std::string &registrationId) {
            return proxy::IListImpl::removeItemListener(registrationId);
        }

        int MixedList::size() {
            return proxy::IListImpl::size();
        }

        bool MixedList::isEmpty() {
            return size() == 0;
        }

        std::vector<TypedData> MixedList::toArray() {
            return toTypedDataCollection(proxy::IListImpl::toArrayData());
        }

        void MixedList::clear() {
            proxy::IListImpl::clear();
        }

        TypedData MixedList::get(int index) {
            return TypedData(proxy::IListImpl::getData(index), context->getSerializationService());
        }

        TypedData MixedList::remove(int index) {
            return TypedData(proxy::IListImpl::removeData(index), context->getSerializationService());
        }

        std::vector<TypedData> MixedList::subList(int fromIndex, int toIndex) {
            return toTypedDataCollection(proxy::IListImpl::subListData(fromIndex, toIndex));
        }
    }
}
