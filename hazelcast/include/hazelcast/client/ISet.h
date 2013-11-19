#ifndef HAZELCAST_ISET
#define HAZELCAST_ISET

#include "hazelcast/client/collection/CollectionAddListenerRequest.h"
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
#include "ItemEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/proxy/DistributedObject.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        template<typename E>
        class ISet : public proxy::DistributedObject {
            friend class HazelcastClient;

        public:

            template < typename L>
            long addItemListener(L &listener, bool includeValue) {
                collection::CollectionAddListenerRequest request(getName(), includeValue);
                request.setServiceName(getServiceName());
                impl::ItemEventHandler<E, L> entryEventHandler(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return getContext().getServerListenerService().template listen<collection::CollectionAddListenerRequest, impl::ItemEventHandler<E, L>, impl::PortableItemEvent >(request, entryEventHandler);
            };

            bool removeItemListener(long registrationId) {
                return getContext().getServerListenerService().stopListening(registrationId);
            };

            int size() {
                collection::CollectionSizeRequest request(getName());
                return invoke<int>(request);
            };

            bool isEmpty() {
                return size() == 0;
            };

            bool contains(const E &o) {
                serialization::Data valueData = toData(o);
                std::vector<serialization::Data> valueSet;
                valueSet.push_back(valueData);
                collection::CollectionContainsRequest request (getName(), valueSet);
                return invoke<bool>(request);
            };

            std::vector<E> toArray() {
                collection::CollectionGetAllRequest request(getName());
                impl::SerializableCollection result = invoke<impl::SerializableCollection>(request);
                const std::vector<serialization::Data *> &collection = result.getCollection();
                std::vector<E> set(collection.size());
                for (int i = 0; i < collection.size(); ++i) {
                    set[i] = toObject<E>(*(collection[i]));
                }
                return set;
            };

            bool add(const E &e) {
                serialization::Data valueData = toData(e);
                collection::CollectionAddRequest request(getName(), valueData);
                return invoke<bool>(request);
            };

            bool remove(const E &e) {
                serialization::Data valueData = toData(e);
                collection::CollectionRemoveRequest request(getName(), valueData);
                return invoke<bool>(request);
            };

            bool containsAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionContainsRequest request(getName(), dataCollection);
                return invoke<bool>(request);
            };

            bool addAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionAddAllRequest request(getName(), dataCollection);
                return invoke<bool>(request);
            };

            bool removeAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionCompareAndRemoveRequest request(getName(), dataCollection, false);
                return invoke<bool>(request);
            };

            bool retainAll(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataCollection = toDataCollection(objects);
                collection::CollectionCompareAndRemoveRequest request(getName(), dataCollection, true);
                return invoke<bool>(request);
            };

            void clear() {
                collection::CollectionClearRequest request(getName());
                invoke<bool>(request);
            };

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */
            void onDestroy() {

            };

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
            T toObject(const serialization::Data &data) {
                return getContext().getSerializationService().template toObject<T>(data);
            };

            template<typename Response, typename Request>
            Response invoke(Request &request) {
                request.setServiceName(getServiceName());
                return getContext().getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            ISet(const std::string &instanceName, spi::ClientContext *clientContext)
            : DistributedObject("hz:impl:setService", instanceName, clientContext)
            , key(toData(instanceName)) {

            };

            serialization::Data key;
        };

    }
}

#endif /* HAZELCAST_ISET */