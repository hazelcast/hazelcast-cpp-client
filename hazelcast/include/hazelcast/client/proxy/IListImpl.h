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
//
// Created by sancar koyunlu on 30/09/14.
//

#ifndef HAZELCAST_IListImpl
#define HAZELCAST_IListImpl


#include "hazelcast/client/proxy/ProxyImpl.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API IListImpl : public ProxyImpl {
            protected:
                IListImpl(const std::string& instanceName, spi::ClientContext *context);

                std::string addItemListener(impl::BaseEventHandler *entryEventHandler, bool includeValue);

                bool removeItemListener(const std::string& registrationId);

                int size();

                bool isEmpty();

                bool contains(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data> toArrayData();

                bool add(const serialization::pimpl::Data& element);

                bool remove(const serialization::pimpl::Data& element);

                bool containsAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(int index, const std::vector<serialization::pimpl::Data>& elements);

                bool removeAll(const std::vector<serialization::pimpl::Data>& elements);

                bool retainAll(const std::vector<serialization::pimpl::Data>& elements);

                void clear();

                std::auto_ptr<serialization::pimpl::Data> getData(int index);

                std::auto_ptr<serialization::pimpl::Data> setData(int index, const serialization::pimpl::Data& element);

                void add(int index, const serialization::pimpl::Data& element);

                std::auto_ptr<serialization::pimpl::Data> removeData(int index);

                int indexOf(const serialization::pimpl::Data& element);

                int lastIndexOf(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data> subListData(int fromIndex, int toIndex);

            private:
                class ListListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ListListenerMessageCodec(const std::string &name, bool includeValue);

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
#endif //HAZELCAST_IListImpl
