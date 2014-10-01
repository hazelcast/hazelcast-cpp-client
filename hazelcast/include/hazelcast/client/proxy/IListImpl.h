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

                bool contains(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data *> toArray();

                bool add(const serialization::pimpl::Data& element);

                bool remove(const serialization::pimpl::Data& element);

                bool containsAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(const std::vector<serialization::pimpl::Data>& elements);

                bool addAll(int index, const std::vector<serialization::pimpl::Data>& elements);

                bool removeAll(const std::vector<serialization::pimpl::Data>& elements);

                bool retainAll(const std::vector<serialization::pimpl::Data>& elements);

                void clear();

                serialization::pimpl::Data get(int index);

                serialization::pimpl::Data set(int index, const serialization::pimpl::Data& element);

                void add(int index, const serialization::pimpl::Data& element);

                serialization::pimpl::Data remove(int index);

                int indexOf(const serialization::pimpl::Data& element);

                int lastIndexOf(const serialization::pimpl::Data& element);

                std::vector<serialization::pimpl::Data *> subList(int fromIndex, int toIndex);

            private:
                int partitionId;
            };
        }
    }
}
#endif //HAZELCAST_IListImpl
