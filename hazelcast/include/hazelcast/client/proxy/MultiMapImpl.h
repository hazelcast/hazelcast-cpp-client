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

#include <string>
#include <vector>
#include <chrono>

#include "hazelcast/client/proxy/ProxyImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientLockReferenceIdGenerator;
        }

        namespace proxy {
            class HAZELCAST_API MultiMapImpl : public ProxyImpl {
            public:
                /**
                * Returns the number of key-value pairs in the multimap.
                *
                * @return the number of key-value pairs in the multimap.
                */
                boost::future<int> size();

                /**
                * Clears the multimap. Removes all key-value pairs.
                */
                boost::future<void> clear();

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                * @param registrationId Id of listener registration
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeEntryListener(boost::uuids::uuid registrationId);
            protected:
                MultiMapImpl(const std::string& instanceName, spi::ClientContext *context);

                boost::future<bool> put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                boost::future<std::vector<serialization::pimpl::Data>> getData(const serialization::pimpl::Data &key);

                boost::future<bool> remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                boost::future<std::vector<serialization::pimpl::Data>> removeData(const serialization::pimpl::Data& key);

                boost::future<std::vector<serialization::pimpl::Data>> keySetData();

                boost::future<std::vector<serialization::pimpl::Data>> valuesData();

                boost::future<EntryVector> entrySetData();

                boost::future<bool> containsKey(const serialization::pimpl::Data& key);

                boost::future<bool> containsValue(const serialization::pimpl::Data& key);

                boost::future<bool> containsEntry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                boost::future<int> valueCount(const serialization::pimpl::Data& key);

                boost::future<boost::uuids::uuid>
                addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler, bool includeValue);

                boost::future<boost::uuids::uuid>
                addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler, bool includeValue,
                                 Data &&key);

                boost::future<void> lock(const serialization::pimpl::Data& key);

                boost::future<void> lock(const serialization::pimpl::Data& key, std::chrono::milliseconds leaseTime);

                boost::future<bool> isLocked(const serialization::pimpl::Data& key);

                boost::future<bool> tryLock(const serialization::pimpl::Data& key);

                boost::future<bool> tryLock(const serialization::pimpl::Data& key, std::chrono::milliseconds timeout);

                boost::future<bool>
                tryLock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout,
                        std::chrono::milliseconds leaseTime);

                boost::future<void> unlock(const serialization::pimpl::Data& key);

                boost::future<void> forceUnlock(const serialization::pimpl::Data& key);

                void onInitialize() override;
            private:
                class MultiMapEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MultiMapEntryListenerMessageCodec(std::string name, bool includeValue);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;
                private:
                    std::string name_;
                    bool includeValue_;
                };

                class MultiMapEntryListenerToKeyCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MultiMapEntryListenerToKeyCodec(std::string name, bool includeValue,
                                                    serialization::pimpl::Data &&key);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;
                private:
                    std::string  name_;
                    bool includeValue_;
                    serialization::pimpl::Data key_;
                };

                std::shared_ptr<impl::ClientLockReferenceIdGenerator> lockReferenceIdGenerator_;

                std::shared_ptr<spi::impl::ListenerMessageCodec> createMultiMapEntryListenerCodec(bool includeValue);

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createMultiMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &&key);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

