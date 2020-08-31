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
#include <memory>
#include <chrono>
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace proxy {
            class HAZELCAST_API IQueueImpl : public ProxyImpl {
            public:
                /**
                * Removes the specified item listener.
                * Returns silently if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeItemListener(
                        boost::uuids::uuid registrationId);

                /**
                *
                * @return size of this distributed queue
                */
                boost::future<int> size();

                /**
                *
                * @return true if queue is empty
                */
                boost::future<bool> isEmpty();

                /**
                *
                * @return remaining capacity
                */
                boost::future<int> remainingCapacity();

                /**
                * Removes all elements from queue.
                */
                boost::future<void> clear();
            protected:
                IQueueImpl(const std::string& instanceName, spi::ClientContext *context);

                boost::future<boost::uuids::uuid>
                addItemListener(std::unique_ptr<impl::ItemEventHandler<protocol::codec::queue_addlistener_handler>> &&itemEventHandler, bool includeValue) {
                    return registerListener(createItemListenerCodec(includeValue), std::move(itemEventHandler));
                }
                
                boost::future<bool> offer(const serialization::pimpl::Data& element, std::chrono::steady_clock::duration timeout);

                boost::future<void> put(const serialization::pimpl::Data& element);

                boost::future<boost::optional<serialization::pimpl::Data>>pollData(std::chrono::steady_clock::duration timeout);

                boost::future<bool> remove(const serialization::pimpl::Data& element);

                boost::future<bool> contains(const serialization::pimpl::Data& element);

                boost::future<std::vector<serialization::pimpl::Data>> drainToData(size_t maxElements);

                boost::future<std::vector<serialization::pimpl::Data>> drainToData();

                boost::future<boost::optional<serialization::pimpl::Data>>peekData();

                boost::future<std::vector<serialization::pimpl::Data>> toArrayData();

                boost::future<bool> containsAllData(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> addAllData(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> removeAllData(const std::vector<serialization::pimpl::Data>& elements);

                boost::future<bool> retainAllData(const std::vector<serialization::pimpl::Data>& elements);

            private:
                class QueueListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    QueueListenerMessageCodec(std::string name, bool includeValue);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;

                private:
                    std::string name;
                    bool includeValue;
                };

                int partitionId;

                std::shared_ptr<spi::impl::ListenerMessageCodec> createItemListenerCodec(bool includeValue);
            };
        }
    }
}

