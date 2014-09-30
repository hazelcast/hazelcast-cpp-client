#ifndef HAZELCAST_IMAP_IMPL
#define HAZELCAST_IMAP_IMPL

#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace pimpl {
            class HAZELCAST_API IMapImpl : public DistributedObject {
            public:
                IMapImpl(const std::string& instanceName, spi::ClientContext *context);

                void onDestroy();

                bool containsKey(const serialization::pimpl::Data& key);

                bool containsValue(const serialization::pimpl::Data& value);

                serialization::pimpl::Data get(const serialization::pimpl::Data& key);

                serialization::pimpl::Data put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                serialization::pimpl::Data remove(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void deleteEntry(const serialization::pimpl::Data& key);

                void flush();

                bool tryRemove(const serialization::pimpl::Data& key, long timeoutInMillis);

                bool tryPut(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long timeoutInMillis);

                serialization::pimpl::Data put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                void putTransient(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                serialization::pimpl::Data putIfAbsent(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                bool replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue);

                serialization::pimpl::Data replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttl);

                void lock(const serialization::pimpl::Data& key);

                void lock(const serialization::pimpl::Data& key, long leaseTime);

                bool isLocked(const serialization::pimpl::Data& key);

                bool tryLock(const serialization::pimpl::Data& key, long timeInMillis);

                void unlock(const serialization::pimpl::Data& key);

                void forceUnlock(const serialization::pimpl::Data& key);

                void removeInterceptor(const std::string& id);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue);

                bool removeEntryListener(const std::string& registrationId);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, const serialization::pimpl::Data& key, bool includeValue);

                map::DataEntryView getEntryView(const serialization::pimpl::Data& key);

                bool evict(const serialization::pimpl::Data& key);

                void evictAll();

                std::vector<serialization::pimpl::Data> keySet();

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > getAll(const std::vector<serialization::pimpl::Data>& keys);

                std::vector<serialization::pimpl::Data> values();

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entrySet();

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > keySet(const std::string& sql);

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entrySet(const std::string& sql);

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > values(const std::string& sql);

                void addIndex(const std::string& attribute, bool ordered);

                int size();

                bool isEmpty();

                void putAll(const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >& m);

                void clear();

            };
        }

    }
}

#endif /* HAZELCAST_IMAP_IMPL */

