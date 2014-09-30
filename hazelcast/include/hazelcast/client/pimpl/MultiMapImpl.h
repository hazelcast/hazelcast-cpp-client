#ifndef HAZELCAST_MULTI_MAP_IMPL
#define HAZELCAST_MULTI_MAP_IMPL


#include "hazelcast/client/DistributedObject.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace pimpl {
            class MultiMapImpl : public hazelcast::client::DistributedObject {
            public:
                MultiMapImpl(const std::string& instanceName, spi::ClientContext *context);

                void onDestroy();

                bool put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::vector<serialization::pimpl::Data> get(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::vector<serialization::pimpl::Data> remove(const serialization::pimpl::Data& key);

                std::vector<serialization::pimpl::Data> keySet();

                std::vector<serialization::pimpl::Data> values();

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entrySet();

                bool containsKey(const serialization::pimpl::Data& key);

                bool containsValue(const serialization::pimpl::Data& key);

                bool containsEntry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                int size();

                void clear();

                int valueCount(const serialization::pimpl::Data& key);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, const serialization::pimpl::Data& key, bool includeValue);

                bool removeEntryListener(const std::string& registrationId);

                void lock(const serialization::pimpl::Data& key);

                void lock(const serialization::pimpl::Data& key, long leaseTimeInMillis);

                bool isLocked(const serialization::pimpl::Data& key);

                bool tryLock(const serialization::pimpl::Data& key);

                bool tryLock(const serialization::pimpl::Data& key, long timeoutInMillis);

                void unlock(const serialization::pimpl::Data& key);

                void forceUnlock(const serialization::pimpl::Data& key);

            };
        }
    }
}

#endif /* HAZELCAST_MULTI_MAP_IMPL */

