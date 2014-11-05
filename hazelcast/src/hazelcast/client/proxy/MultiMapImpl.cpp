//
// Created by sancar koyunlu on 30/09/14.
//

#include "hazelcast/client/proxy/MultiMapImpl.h"
#include "hazelcast/client/multimap/PutRequest.h"
#include "hazelcast/client/multimap/RemoveRequest.h"
#include "hazelcast/client/multimap/RemoveAllRequest.h"
#include "hazelcast/client/multimap/KeySetRequest.h"
#include "hazelcast/client/multimap/ValuesRequest.h"
#include "hazelcast/client/multimap/EntrySetRequest.h"
#include "hazelcast/client/multimap/SizeRequest.h"
#include "hazelcast/client/multimap/ClearRequest.h"
#include "hazelcast/client/multimap/CountRequest.h"
#include "hazelcast/client/multimap/AddEntryListenerRequest.h"
#include "hazelcast/client/multimap/RemoveEntryListenerRequest.h"
#include "hazelcast/client/multimap/KeyBasedContainsRequest.h"
#include "hazelcast/client/multimap/ContainsRequest.h"
#include "hazelcast/client/multimap/GetAllRequest.h"
#include "hazelcast/client/multimap/MultiMapLockRequest.h"
#include "hazelcast/client/multimap/MultiMapUnlockRequest.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/multimap/MultiMapIsLockedRequest.h"
#include "hazelcast/client/multimap/PortableEntrySetResponse.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"


namespace hazelcast {
    namespace client {
        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:multiMapService", instanceName, context) {

            }

            bool MultiMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                multimap::PutRequest *request = new multimap::PutRequest(getName(), key, value, -1, util::getThreadId());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::get(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::GetAllRequest *request = new multimap::GetAllRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, impl::PortableCollection);
                return result->getCollection();
            }

            bool MultiMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                multimap::RemoveRequest *request = new multimap::RemoveRequest(getName(), key, value, util::getThreadId());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::remove(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::RemoveAllRequest *request = new multimap::RemoveAllRequest(getName(), key, util::getThreadId());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, impl::PortableCollection);
                return result->getCollection();
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::keySet() {
                multimap::KeySetRequest *request = new multimap::KeySetRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, impl::PortableCollection);
                return result->getCollection();
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::values() {
                multimap::ValuesRequest *request = new multimap::ValuesRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, impl::PortableCollection);
                return result->getCollection();
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > MultiMapImpl::entrySet() {
                multimap::EntrySetRequest *request = new multimap::EntrySetRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, multimap::PortableEntrySetResponse);
                return result->getEntrySet();
            }

            bool MultiMapImpl::containsKey(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::KeyBasedContainsRequest *request = new multimap::KeyBasedContainsRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool MultiMapImpl::containsValue(const serialization::pimpl::Data& value) {
                multimap::ContainsRequest *request = new multimap::ContainsRequest(getName(), value);
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool MultiMapImpl::containsEntry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                multimap::KeyBasedContainsRequest *request = new multimap::KeyBasedContainsRequest(getName(), key, value);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            int MultiMapImpl::size() {
                multimap::SizeRequest *request = new multimap::SizeRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, int);
                return *result;
            }

            void MultiMapImpl::clear() {
                multimap::ClearRequest *request = new multimap::ClearRequest(getName());
                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::CountRequest *request = new multimap::CountRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int);
                return *result;
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                multimap::AddEntryListenerRequest *request = new multimap::AddEntryListenerRequest(getName(), includeValue);
                return listen(request, entryEventHandler);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, const serialization::pimpl::Data& key, bool includeValue) {
                int partitionId = getPartitionId(key);
                multimap::AddEntryListenerRequest *request = new multimap::AddEntryListenerRequest(getName(), key, includeValue);
                return listen(request, partitionId, entryEventHandler);
            }

            bool MultiMapImpl::removeEntryListener(const std::string& registrationId) {
                multimap::RemoveEntryListenerRequest *request = new multimap::RemoveEntryListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapLockRequest *request = new multimap::MultiMapLockRequest(getName(), key, util::getThreadId());
                invoke(request, partitionId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key, long leaseTimeInMillis) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapLockRequest *request = new multimap::MultiMapLockRequest(getName(), key, util::getThreadId(), leaseTimeInMillis, -1);
                invoke(request, partitionId);
            }


            bool MultiMapImpl::isLocked(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapIsLockedRequest *request = new multimap::MultiMapIsLockedRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapLockRequest *request = new multimap::MultiMapLockRequest(getName(), key, util::getThreadId(), -1, 0);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key, long timeoutInMillis) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapLockRequest *request = new multimap::MultiMapLockRequest(getName(), key, util::getThreadId(), -1, timeoutInMillis);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            void MultiMapImpl::unlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapUnlockRequest *request = new multimap::MultiMapUnlockRequest(getName(), key, util::getThreadId());
                invoke(request, partitionId);
            }

            void MultiMapImpl::forceUnlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                multimap::MultiMapUnlockRequest *request = new multimap::MultiMapUnlockRequest(getName(), key, util::getThreadId(), true);
                invoke(request, partitionId);
            }
        }
    }
}

