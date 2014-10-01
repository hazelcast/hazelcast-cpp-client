#ifndef HAZELCAST_IQUEUE_IMPL
#define HAZELCAST_IQUEUE_IMPL

#include "hazelcast/client/proxy/ProxyImpl.h"
#include <vector>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class IQueueImpl : public ProxyImpl {
            protected:
                IQueueImpl(const std::string& instanceName, spi::ClientContext *context);

                std::string addItemListener(impl::BaseEventHandler *handler, bool includeValue);

                bool removeItemListener(const std::string& registrationId);

                bool offer(const serialization::pimpl::Data& element, long timeoutInMillis);

                serialization::pimpl::Data poll(long timeoutInMillis);

                int remainingCapacity();

                bool remove(const serialization::pimpl::Data& element);

                bool contains(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data>  drainTo(int maxElements);

                serialization::pimpl::Data peek();

                int size();

                std::vector<serialization::pimpl::Data> toArray();

                bool containsAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(const std::vector<serialization::pimpl::Data>& elements);

                bool removeAll(const std::vector<serialization::pimpl::Data>& elements);

                bool retainAll(const std::vector<serialization::pimpl::Data>& elements);

                void clear();

            private:

                int partitionId;


            };
        }
    }
}

#endif /* HAZELCAST_IQUEUE */

