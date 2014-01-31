#ifndef HAZELCAST_ISET
#define HAZELCAST_ISET

#include "hazelcast/client/collection/CollectionAddListenerRequest.h"
#include "hazelcast/client/collection/CollectionRemoveListenerRequest.h"
#include "hazelcast/client/collection/CollectionSizeRequest.h"
#include "hazelcast/client/collection/CollectionContainsRequest.h"
#include "hazelcast/client/collection/CollectionRemoveRequest.h"
#include "hazelcast/client/collection/CollectionAddAllRequest.h"
#include "hazelcast/client/collection/CollectionCompareAndRemoveRequest.h"
#include "hazelcast/client/collection/CollectionGetAllRequest.h"
#include "hazelcast/client/collection/CollectionAddRequest.h"
#include "hazelcast/client/collection/CollectionClearRequest.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/DistributedObject.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        template<typename E>
        class HAZELCAST_API ISet : public DistributedObject {
            friend class HazelcastClient;

        public:

            template < typename L>
            std::string addItemListener(L &listener, bool includeValue) {
                collection::CollectionAddListenerRequest *request = new collection::CollectionAddListenerRequest(getName(), getServiceName(), includeValue);
                impl::ItemEventHandler<E, L> *entryEventHandler = new impl::ItemEventHandler<E, L>(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return listen(request, entryEventHandler);
            };

            bool removeItemListener(const std::string &registrationId) {
                collection::CollectionRemoveListenerRequest *request = new collection::CollectionRemoveListenerRequest(getName(), getServiceName(), registrationId);
                return stopListening(request, registrationId);
            };

            int size() {
                collection::CollectionSizeRequest *request = new collection::CollectionSizeRequest(getName(), getServiceName());
                boost::shared_ptr<int> i = invoke<int>(request, partitionId);
                return *i;
            };

            bool isEmpty() {
                return size() == 0;
            };

            bool contains(const E &o) {
                serialization::Data valueData = toData(o);
                std::vector<serialization::Data> valueSet;
                valueSet.push_back(valueData);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest (getName(), getServiceName(), valueSet);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            std::vector<E> toArray() {
                collection::CollectionGetAllRequest *request = new collection::CollectionGetAllRequest(getName(), getServiceName());
                boost::shared_ptr<impl::SerializableCollection> result = invoke<impl::SerializableCollection>(request, partitionId);
                const std::vector<serialization::Data *> &collection = result->getCollection();
                std::vector<E> set(collection.size());
                for (int i = 0; i < collection.size(); ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*(collection[i]));
                    set[i] = *e;
                }
                return set;
            };

            bool add(const E &e) {
                serialization::Data valueData = toData(e);
                collection::CollectionAddRequest *request = new collection::CollectionAddRequest(getName(), getServiceName(), valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            bool remove(const E &e) {
                serialization::Data valueData = toData(e);
                collection::CollectionRemoveRequest *request = new collection::CollectionRemoveRequest(getName(), getServiceName(), valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            bool containsAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), dataCollection);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            bool addAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionAddAllRequest *request = new collection::CollectionAddAllRequest(getName(), getServiceName(), dataCollection);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            bool removeAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), dataCollection, false);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            bool retainAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), dataCollection, true);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            void clear() {
                collection::CollectionClearRequest *request = new collection::CollectionClearRequest(getName(), getServiceName());
                invoke<bool>(request, partitionId);
            };

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */

        private:
            template<typename T>
            const std::vector<serialization::Data> toDataCollection(const std::vector<T> &objects) {
                std::vector<serialization::Data> dataCollection(objects.size());
                for (int i = 0; i < objects.size(); ++i) {
                    dataCollection[i] = toData(objects[i]);
                }
                return dataCollection;
            };

            template<typename T>
            serialization::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename T>
            boost::shared_ptr<T> toObject(const serialization::Data &data) {
                return getContext().getSerializationService().template toObject<T>(data);
            };

            ISet(const std::string &instanceName, spi::ClientContext *clientContext)
            : DistributedObject("hz:impl:setService", instanceName, clientContext) {
                serialization::Data keyData = toData(instanceName);
                partitionId = getPartitionId(keyData);
            };

            void onDestroy() {

            };

            int partitionId;
        };

    }
}

#endif /* HAZELCAST_ISET */