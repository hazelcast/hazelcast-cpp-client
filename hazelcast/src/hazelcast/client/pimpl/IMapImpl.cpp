//
// Created by sancar koyunlu on 29/09/14.
//

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/MapKeySet.h"
#include "hazelcast/client/impl/MapEntrySet.h"
#include "hazelcast/client/impl/MapValueCollection.h"
#include "hazelcast/client/map/GetRequest.h"
#include "hazelcast/client/map/PutRequest.h"
#include "hazelcast/client/map/RemoveRequest.h"
#include "hazelcast/client/map/ContainsKeyRequest.h"
#include "hazelcast/client/map/ContainsValueRequest.h"
#include "hazelcast/client/map/FlushRequest.h"
#include "hazelcast/client/map/RemoveIfSameRequest.h"
#include "hazelcast/client/map/DeleteRequest.h"
#include "hazelcast/client/map/TryRemoveRequest.h"
#include "hazelcast/client/map/TryPutRequest.h"
#include "hazelcast/client/map/PutTransientRequest.h"
#include "hazelcast/client/map/ReplaceIfSameRequest.h"
#include "hazelcast/client/map/ReplaceRequest.h"
#include "hazelcast/client/map/SetRequest.h"
#include "hazelcast/client/map/LockRequest.h"
#include "hazelcast/client/map/IsLockedRequest.h"
#include "hazelcast/client/map/UnlockRequest.h"
#include "hazelcast/client/map/GetEntryViewRequest.h"
#include "hazelcast/client/map/EvictRequest.h"
#include "hazelcast/client/map/EvictAllRequest.h"
#include "hazelcast/client/map/KeySetRequest.h"
#include "hazelcast/client/map/GetAllRequest.h"
#include "hazelcast/client/map/EntrySetRequest.h"
#include "hazelcast/client/map/ValuesRequest.h"
#include "hazelcast/client/map/AddIndexRequest.h"
#include "hazelcast/client/map/SizeRequest.h"
#include "hazelcast/client/map/ClearRequest.h"
#include "hazelcast/client/map/PutAllRequest.h"
#include "hazelcast/client/map/QueryRequest.h"
#include "hazelcast/client/map/AddEntryListenerRequest.h"
#include "hazelcast/client/map/ExecuteOnKeyRequest.h"
#include "hazelcast/client/map/ExecuteOnAllKeysRequest.h"
#include "hazelcast/client/map/RemoveInterceptorRequest.h"
#include "hazelcast/client/map/PutIfAbsentRequest.h"
#include "hazelcast/client/map/RemoveEntryListenerRequest.h"
#include "hazelcast/client/map/MapIsEmptyRequest.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/impl/QueryResultSet.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/pimpl/IMapImpl.h"


namespace hazelcast {
    namespace client {
        namespace pimpl {

            IMapImpl::IMapImpl(const std::string& instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:mapService", instanceName, context) {

            }

            void IMapImpl::onDestroy() {
            }

            bool IMapImpl::containsKey(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::ContainsKeyRequest *request = new map::ContainsKeyRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool IMapImpl::containsValue(const serialization::pimpl::Data& value) {
                map::ContainsValueRequest *request = new map::ContainsValueRequest(getName(), value);
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, bool);
                return *result;
            }

            serialization::pimpl::Data IMapImpl::get(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::GetRequest *request = new map::GetRequest(getName(), key);
                return invoke(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                map::PutRequest *request = new map::PutRequest(getName(), key, value, util::getThreadId(), 0);
                return invoke(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::remove(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::RemoveRequest *request = new map::RemoveRequest(getName(), key, util::getThreadId());
                return invoke(request, partitionId);
            }

            bool IMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                map::RemoveIfSameRequest *request = new map::RemoveIfSameRequest(getName(), key, value, util::getThreadId());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            void IMapImpl::deleteEntry(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::DeleteRequest *request = new map::DeleteRequest(getName(), key, util::getThreadId());
                invoke(request, partitionId);
            }

            void IMapImpl::flush() {
                map::FlushRequest *request = new map::FlushRequest(getName());
                invoke(request);
            }

            bool IMapImpl::tryRemove(const serialization::pimpl::Data& key, long timeoutInMillis) {
                int partitionId = getPartitionId(key);
                map::TryRemoveRequest *request = new map::TryRemoveRequest(getName(), key, util::getThreadId(), timeoutInMillis);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool IMapImpl::tryPut(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long timeoutInMillis) {
                int partitionId = getPartitionId(key);
                map::TryPutRequest *request = new map::TryPutRequest(getName(), key, value, util::getThreadId(), timeoutInMillis);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            serialization::pimpl::Data IMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis) {
                int partitionId = getPartitionId(key);
                map::PutRequest *request = new map::PutRequest(getName(), key, value, util::getThreadId(), ttlInMillis);
                return invoke(request, partitionId);
            }

            void IMapImpl::putTransient(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis) {
                int partitionId = getPartitionId(key);
                map::PutTransientRequest *request = new map::PutTransientRequest(getName(), key, value, util::getThreadId(), ttlInMillis);
                invoke(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::putIfAbsent(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis) {
                int partitionId = getPartitionId(key);
                map::PutIfAbsentRequest *request = new map::PutIfAbsentRequest(getName(), key, value, util::getThreadId(), ttlInMillis);
                return invoke(request, partitionId);
            }

            bool IMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue) {
                int partitionId = getPartitionId(key);
                map::ReplaceIfSameRequest *request = new map::ReplaceIfSameRequest(getName(), key, oldValue, newValue, util::getThreadId());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            serialization::pimpl::Data IMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                map::ReplaceRequest *request = new map::ReplaceRequest(getName(), key, value, util::getThreadId());
                return invoke(request, partitionId);
            }

            void IMapImpl::set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttl) {
                int partitionId = getPartitionId(key);
                map::SetRequest *request = new map::SetRequest(getName(), key, value, util::getThreadId(), ttl);
                invoke(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::LockRequest *request = new map::LockRequest(getName(), key, util::getThreadId());
                invoke(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data& key, long leaseTime) {
                int partitionId = getPartitionId(key);
                map::LockRequest *request = new map::LockRequest(getName(), key, util::getThreadId(), leaseTime, -1);
                invoke(request, partitionId);
            }

            bool IMapImpl::isLocked(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::IsLockedRequest *request = new map::IsLockedRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool IMapImpl::tryLock(const serialization::pimpl::Data& key, long timeInMillis) {
                int partitionId = getPartitionId(key);
                map::LockRequest *request = new map::LockRequest(getName(), key, util::getThreadId(), LONG_MAX, timeInMillis);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            void IMapImpl::unlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::UnlockRequest *request = new map::UnlockRequest(getName(), key, util::getThreadId(), false);
                invoke(request, partitionId);
            }

            void IMapImpl::forceUnlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::UnlockRequest *request = new map::UnlockRequest(getName(), key, util::getThreadId(), true);
                invoke(request, partitionId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                map::AddEntryListenerRequest *request = new map::AddEntryListenerRequest(getName(), includeValue);
                return listen(request, entryEventHandler);
            }

            bool IMapImpl::removeEntryListener(const std::string& registrationId) {
                map::RemoveEntryListenerRequest *request = new map::RemoveEntryListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, const serialization::pimpl::Data& key, bool includeValue) {
                int partitionId = getPartitionId(key);
                map::AddEntryListenerRequest *request = new map::AddEntryListenerRequest(getName(), includeValue, key);
                return listen(request, partitionId, entryEventHandler);
            }

            map::DataEntryView IMapImpl::getEntryView(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::GetEntryViewRequest *request = new map::GetEntryViewRequest(getName(), key);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, map::DataEntryView);
                return *result;
            }

            bool IMapImpl::evict(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                map::EvictRequest *request = new map::EvictRequest(getName(), key, util::getThreadId());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool);
                return *result;
            }

            void IMapImpl::evictAll() {
                map::EvictAllRequest *request = new map::EvictAllRequest(getName());
                invoke(request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySet() {
                map::KeySetRequest *request = new map::KeySetRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, map::MapKeySet);
                return result->getKeySet();
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > IMapImpl::getAll(const std::vector<serialization::pimpl::Data>& keys) {
                map::GetAllRequest *request = new map::GetAllRequest(getName(), keys);
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, map::MapEntrySet);
                return result->getEntrySet();
            }

            std::vector<serialization::pimpl::Data> IMapImpl::values() {
                map::ValuesRequest *request = new map::ValuesRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, map::MapValueCollection);
                return result->getValues();
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > IMapImpl::entrySet() {
                map::EntrySetRequest *request = new map::EntrySetRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, map::MapEntrySet);
                return result->getEntrySet();
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > IMapImpl::keySet(const std::string& sql) {
                std::string iterationType = "KEY";
                map::QueryRequest *request = new map::QueryRequest(getName(), iterationType, sql);
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, impl::QueryResultSet);
                return result->getResultData();
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > IMapImpl::entrySet(const std::string& sql) {
                std::string iterationType = "ENTRY";
                map::QueryRequest *request = new map::QueryRequest(getName(), iterationType, sql);
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, impl::QueryResultSet);
                return result->getResultData();
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > IMapImpl::values(const std::string& sql) {
                std::string iterationType = "VALUE";
                map::QueryRequest *request = new map::QueryRequest(getName(), iterationType, sql);
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, impl::QueryResultSet);
                return result->getResultData();
            }

            void IMapImpl::addIndex(const std::string& attribute, bool ordered) {
                map::AddIndexRequest *request = new map::AddIndexRequest(getName(), attribute, ordered);
                invoke(request);
            }

            int IMapImpl::size() {
                map::SizeRequest *request = new map::SizeRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, bool);
                return *result;
            }

            bool IMapImpl::isEmpty() {
                map::MapIsEmptyRequest *request = new map::MapIsEmptyRequest(getName());
                serialization::pimpl::Data data = invoke(request);
                DESERIALIZE(data, bool);
                return *result;
            }

            void IMapImpl::putAll(const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >& entrySet) {
                map::PutAllRequest *request = new map::PutAllRequest(getName(), entrySet);
                invoke(request);
            }

            void IMapImpl::clear() {
                map::ClearRequest *request = new map::ClearRequest(getName());
                invoke(request);
            }

            void IMapImpl::removeInterceptor(const std::string& id) {
                map::RemoveInterceptorRequest *request = new map::RemoveInterceptorRequest(getName(), id);
                invoke(request);
            }

        }
    }
}