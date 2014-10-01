//
// Created by sancar koyunlu on 30/09/14.
//

#ifndef HAZELCAST_ISetImpl
#define HAZELCAST_ISetImpl


#include "hazelcast/client/proxy/ProxyImpl.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class ISetImpl : public ProxyImpl {
            protected:
                ISetImpl(const std::string& instanceName, spi::ClientContext *clientContext);

                std::string addItemListener(impl::BaseEventHandler *handler, bool includeValue);

                bool removeItemListener(const std::string& registrationId);

                int size();

                bool contains(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data *> toArray();

                bool add(const serialization::pimpl::Data& element);

                bool remove(const serialization::pimpl::Data& element);

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

#endif //HAZELCAST_ISetImpl
