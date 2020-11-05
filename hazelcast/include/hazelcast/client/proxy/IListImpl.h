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

#include <vector>

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API IListImpl : public ProxyImpl {
            public:
                /**
                * Removes the specified item listener.
                * Returns false if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeItemListener(boost::uuids::uuid registrationId);

                /**
                *
                * @return size of the distributed list
                */
                boost::future<int32_t> size();

                /**
                *
                * @return true if empty
                */
                boost::future<bool> isEmpty();

                /**
                * Removes all elements from list.
                */
                boost::future<void> clear();
            protected:
                IListImpl(const std::string& instanceName, spi::ClientContext *context);

                boost::future<boost::uuids::uuid>
                addItemListener(std::unique_ptr<impl::ItemEventHandler<protocol::codec::list_addlistener_handler>> &&itemEventHandler, bool includeValue) {
                    return registerListener(createItemListenerCodec(includeValue), std::move(itemEventHandler));
                }

                boost::future<bool> contains(const serialization::pimpl::Data& element);

                boost::future<std::vector<serialization::pimpl::Data>> toArrayData();

                boost::future<bool> add(const serialization::pimpl::Data& element);

                boost::future<bool> remove(const serialization::pimpl::Data& element);

                boost::future<bool> containsAllData(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> addAllData(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> addAllData(int32_t index, const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> removeAllData(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> retainAllData(const std::vector<serialization::pimpl::Data>& elements);
                
                 boost::future<boost::optional<serialization::pimpl::Data>> getData(int32_t index);

                 boost::future<boost::optional<serialization::pimpl::Data>> setData(int32_t index, const serialization::pimpl::Data& element);

                boost::future<void> add(int32_t index, const serialization::pimpl::Data& element);

                 boost::future<boost::optional<serialization::pimpl::Data>> removeData(int32_t index);

                boost::future<int32_t> indexOf(const serialization::pimpl::Data& element);

                boost::future<int32_t> lastIndexOf(const serialization::pimpl::Data& element);

                boost::future<std::vector<serialization::pimpl::Data>> subListData(int32_t fromIndex, int32_t toIndex);
            private:
                class ListListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ListListenerMessageCodec(std::string name, bool includeValue);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;
                private:
                    std::string name_;
                    bool includeValue_;
                };

                int partitionId_;

                std::shared_ptr<spi::impl::ListenerMessageCodec> createItemListenerCodec(bool includeValue);
            };
        }
    }
}
