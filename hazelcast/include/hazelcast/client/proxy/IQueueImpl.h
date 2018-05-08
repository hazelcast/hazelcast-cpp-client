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
#ifndef HAZELCAST_IQUEUE_IMPL
#define HAZELCAST_IQUEUE_IMPL

#include "hazelcast/client/proxy/ProxyImpl.h"
#include <vector>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace proxy {
            class HAZELCAST_API IQueueImpl : public ProxyImpl {
            protected:
                IQueueImpl(const std::string& instanceName, spi::ClientContext *context);

                std::string addItemListener(impl::BaseEventHandler *handler, bool includeValue);

                bool removeItemListener(const std::string& registrationId);

                bool offer(const serialization::pimpl::Data& element, long timeoutInMillis);

                void put(const serialization::pimpl::Data& element);

                std::auto_ptr<serialization::pimpl::Data> pollData(long timeoutInMillis);

                int remainingCapacity();

                bool remove(const serialization::pimpl::Data& element);

                bool contains(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data>  drainToData(size_t maxElements);

                std::auto_ptr<serialization::pimpl::Data> peekData();

                int size();

                bool isEmpty();

                std::vector<serialization::pimpl::Data> toArrayData();

                bool containsAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(const std::vector<serialization::pimpl::Data>& elements);

                bool removeAll(const std::vector<serialization::pimpl::Data>& elements);

                bool retainAll(const std::vector<serialization::pimpl::Data>& elements);

                void clear();
            private:
                class QueueListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    QueueListenerMessageCodec(const std::string &name, bool includeValue);

                    virtual std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::auto_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                };

                int partitionId;

                boost::shared_ptr<spi::impl::ListenerMessageCodec> createItemListenerCodec(bool includeValue);
            };
        }
    }
}

#endif /* HAZELCAST_IQUEUE */

