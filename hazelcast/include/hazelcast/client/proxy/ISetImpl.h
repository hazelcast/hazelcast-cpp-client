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

#include "hazelcast/client/proxy/ProxyImpl.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ISetImpl : public ProxyImpl {
            public:
                /**
                * Removes the specified item listener.
                * Returns false if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeItemListener(const std::string& registrationId);

                /**
                *
                * @returns size of the distributed set
                */
                boost::future<int> size();

                /**
                *
                * @returns true if empty
                */
                boost::future<bool> isEmpty();

                /**
                *
                * Removes all elements from set.
                */
                boost::future<void> clear();
            protected:
                ISetImpl(const std::string& instanceName, spi::ClientContext *clientContext);

                template<typename Listener>
                boost::future<std::string>
                addItemListener(std::unique_ptr<impl::ItemEventHandler<Listener, protocol::codec::SetAddListenerCodec::AbstractEventHandler>> &&itemEventHandler, bool includeValue) {
                    return registerListener(createItemListenerCodec(includeValue), std::move(itemEventHandler));
                }

                boost::future<bool> contains(const serialization::pimpl::Data& element);

                boost::future<std::vector<serialization::pimpl::Data>> toArrayData();

                boost::future<bool> add(const serialization::pimpl::Data& element);

                boost::future<bool> remove(const serialization::pimpl::Data& element);

                boost::future<bool> containsAll(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> addAll(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> removeAll(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> retainAll(const std::vector<serialization::pimpl::Data>& elements);

            private:
                class SetListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    SetListenerMessageCodec(std::string name, bool includeValue);

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override;

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override;

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override;

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override;

                private:
                    std::string  name;
                    bool includeValue;
                };

                int partitionId;

                std::unique_ptr<spi::impl::ListenerMessageCodec> createItemListenerCodec(bool includeValue);
            };
        }
    }
}
