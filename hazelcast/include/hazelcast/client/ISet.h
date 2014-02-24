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
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/DistributedObject.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        /**
         * Concurrent, distributed client implementation of std::unordered_set.
         *
         * @param <E> item type
         */
        template<typename E>
        class HAZELCAST_API ISet : public DistributedObject {
            friend class HazelcastClient;

        public:
            /**
             * Listener should implement itemAdded and itemRemoved as follows
             *
             *      class MyListItemListener {
             *      public:
             *      //....
             *
             *      void itemAdded(ItemEvent<std::string> itemEvent) {
             *          //...
             *      }
             *
             *      void itemRemoved(ItemEvent<std::string> item) {
             *              //...
             *      }
             *
             *      };
             *
             *  Note that E is std::string in the example
             *
             *  @param L listener
             *  @param bool includeValue should ItemEvent include value or not.
             */
            template < typename L>
            std::string addItemListener(L &listener, bool includeValue) {
                collection::CollectionAddListenerRequest *request = new collection::CollectionAddListenerRequest(getName(), getServiceName(), includeValue);
                impl::ItemEventHandler<E, L> *entryEventHandler = new impl::ItemEventHandler<E, L>(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return listen(request, entryEventHandler);
            };

            /**
             * Removes the specified item listener.
             * Returns false if the specified listener is not added before.
             *
             * @param registrationId Id of listener registration.
             *
             * @return true if registration is removed, false otherwise
             */
            bool removeItemListener(const std::string &registrationId) {
                collection::CollectionRemoveListenerRequest *request = new collection::CollectionRemoveListenerRequest(getName(), getServiceName(), registrationId);
                return stopListening(request, registrationId);
            };

            /**
             *
             * @returns size of the distributed set
             */
            int size() {
                collection::CollectionSizeRequest *request = new collection::CollectionSizeRequest(getName(), getServiceName());
                boost::shared_ptr<int> i = invoke<int>(request, partitionId);
                return *i;
            };

            /**
             *
             * @returns true if empty
             */
            bool isEmpty() {
                return size() == 0;
            };

            /**
             *
             * @param element
             * @returns true if set contains element
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool contains(const E &o) {
                serialization::pimpl::Data valueData = toData(o);
                std::vector<serialization::pimpl::Data> valueSet;
                valueSet.push_back(valueData);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest (getName(), getServiceName(), valueSet);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * @returns all elements as std::vector
             */
            std::vector<E> toArray() {
                collection::CollectionGetAllRequest *request = new collection::CollectionGetAllRequest(getName(), getServiceName());
                boost::shared_ptr<impl::SerializableCollection> result = invoke<impl::SerializableCollection>(request, partitionId);
                const std::vector<serialization::pimpl::Data *> &collection = result->getCollection();
                std::vector<E> set(collection.size());
                for (int i = 0; i < collection.size(); ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*(collection[i]));
                    set[i] = *e;
                }
                return set;
            };

            /**
             *
             * @param E e
             * @return true if element is added successfully. If elements was already there returns false.
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool add(const E &e) {
                serialization::pimpl::Data valueData = toData(e);
                collection::CollectionAddRequest *request = new collection::CollectionAddRequest(getName(), getServiceName(), valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * @param E e
             * @return true if element is removed successfully.
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool remove(const E &e) {
                serialization::pimpl::Data valueData = toData(e);
                collection::CollectionRemoveRequest *request = new collection::CollectionRemoveRequest(getName(), getServiceName(), valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * @param elements std::vector<E>
             * @return true if this set contains all elements given in vector.
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool containsAll(const std::vector<E> &objects) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(objects);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), dataCollection);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * @param elements std::vector<E>
             * @return true if all elements given in vector can be added to set.
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool addAll(const std::vector<E> &objects) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(objects);
                collection::CollectionAddAllRequest *request = new collection::CollectionAddAllRequest(getName(), getServiceName(), dataCollection);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * @param elements std::vector<E>
             * @return true if all elements are removed successfully.
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool removeAll(const std::vector<E> &objects) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(objects);
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), dataCollection, false);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * Removes the elements from this set that are not available in given "elements" vector
             * @param elements std::vector<E>
             * @return true if operation is successful.
             * @throws IClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool retainAll(const std::vector<E> &objects) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(objects);
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), dataCollection, true);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
             *
             * Removes all elements from set.
             */
            void clear() {
                collection::CollectionClearRequest *request = new collection::CollectionClearRequest(getName(), getServiceName());
                invoke<serialization::pimpl::Void>(request, partitionId);
            };


        private:
            template<typename T>
            const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T> &objects) {
                std::vector<serialization::pimpl::Data> dataCollection(objects.size());
                for (int i = 0; i < objects.size(); ++i) {
                    dataCollection[i] = toData(objects[i]);
                }
                return dataCollection;
            };

            template<typename T>
            serialization::pimpl::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename T>
            boost::shared_ptr<T> toObject(const serialization::pimpl::Data &data) {
                return getContext().getSerializationService().template toObject<T>(data);
            };

            ISet(const std::string &instanceName, spi::ClientContext *clientContext)
            : DistributedObject("hz:impl:setService", instanceName, clientContext) {
                serialization::pimpl::Data keyData = toData(instanceName);
                partitionId = getPartitionId(keyData);
            };

            void onDestroy() {

            };

            int partitionId;
        };

    }
}

#endif /* HAZELCAST_ISET */