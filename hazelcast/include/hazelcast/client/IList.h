#ifndef HAZELCAST_ILIST
#define HAZELCAST_ILIST

#include "hazelcast/client/spi/DistributedObjectListenerService.h"
#include "CollectionAddListenerRequest.h"
#include "ListAddRequest.h"
#include "ListRemoveRequest.h"
#include "ListAddAllRequest.h"
#include "ListGetRequest.h"
#include "ListSetRequest.h"
#include "ListIndexOfRequest.h"
#include "ListSubRequest.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        template<typename E>
        class IList {
            friend class HazelcastClient;

        public:

            template < typename L>
            long addItemListener(L& listener, bool includeValue) {
                collection::CollectionAddListenerRequest request(name, includeValue);
                impl::ItemEvent<E> entryEventHandler(name, context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return context->getServerListenerService().template listen<queue::AddListenerRequest, impl::ItemEventHandler<E, L>, impl::PortableItemEvent >(request, entryEventHandler);
            };

            bool removeItemListener(long registrationId) {
                return context->getServerListenerService().stopListening(registrationId);
            };

            int size() {
                collection::CollectionSizeRequest request(name);
                return invoke<int>(request);
            };

            bool isEmpty() {
                return size() == 0;
            };

            bool contains(const E& o) {
                serialization::Data valueData = toData(o);
                collection::CollectionContainsRequest request (name, key, valueData);
                return invoke<bool>(request);
            };

            std::vector<E> toArray() {
                collection::CollectionGetAllRequest request(name, key);
                impl::PortableCollection result = invoke<impl::PortableCollection>(request);
                const std::vector<serialization::Data>& collection = result.getCollection();
                std::vector<E> set(collection.size());
                for (int i = 0; i < collection.size(); ++i) {
                    set[i] = toObject<E>(collection[i]);
                }
                return set;
            };

            bool add(const E& e) {
                serialization::Data valueData = toData(e);
                list::ListAddRequest request(name, key, valueData, -1, util::getThreadId());
                return invoke<E>(request);
            };

            bool remove(const E& e) {
                serialization::Data valueData = toData(e);
                list::ListRemoveRequest request(name, key, valueData, util::getThreadId());
                return invoke<bool>(request);
            };

            bool containsAll(const std::vector<E>& objects) {
                collection::CollectionContainsRequest request(name, key, toDataCollection(objects));
                return invoke<bool>(request);
            };

            bool addAll(const std::vector<E>& objects) {
                list::ListAddAllRequest request(name, key, util::getThreadId(), toDataCollection(objects));
                return invoke<bool>(request);
            };

            bool addAll(int index, const std::vector<E>& objects) {
                list::ListAddAllRequest request(name, key, util::getThreadId(), toDataCollection(objects), index);
                return invoke<bool>(request);
            };

            bool removeAll(const std::vector<E>& objects) {
                collection::CollectionCompareAndRemoveRequest request(name, key, util::getThreadId(), false, toDataCollection(objects));
                return invoke<bool>(request);
            };

            bool retainAll(const std::vector<E>& objects) {
                collection::CollectionCompareAndRemoveRequest request(name, key, util::getThreadId(), true, toDataCollection(objects));
                return invoke<bool>(request);
            };

            void clear() {
                collection::CollectionClearRequest request(name, key, util::getThreadId());
                invoke<bool>(request);
            };

            E get(int index) {
                list::ListGetRequest request(name, key, index);
                return invoke<E>(request);
            };

            E set(int index, const E& e) {
                serialization::Data valueData = toData(e);
                list::ListSetRequest request(name, key, valueData, index, util::getThreadId());
                return invoke<E>(request);
            };

            void add(int index, const E& e) {
                serialization::Data valueData = toData(e);
                list::ListAddRequest request(name, key, valueData, index, util::getThreadId());
                invoke<bool>(request);
            };

            E remove(int index) {
                list::ListRemoveRequest request(name, key, index, util::getThreadId());
                return invoke<E>(request);
            };

            int indexOf(const E& e) {
                serialization::Data valueData = toData(e);
                list::ListIndexOfRequest request(name, key, valueData, false);
                return invoke<int>(request);
            };

            int lastIndexOf(const E& e) {
                serialization::Data valueData = toData(e);
                list::ListIndexOfRequest request(name, key, valueData, true);
                return invoke<int>(request);
            };

            std::vector<E> subList(int fromIndex, int toIndex) {
                list::ListSubRequest request(name, key);
                impl::PortableCollection result = invoke<impl::PortableCollection>(request);
                const std::vector<serialization::Data>& collection = result.getCollection();
                std::vector<E> set(fromIndex - toIndex);
                for (int i = fromIndex; i < collection.size(); ++toIndex) {
                    set[fromIndex - i] = toObject<E>(collection[i]);
                }
                return set;
            };

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */
            void destroy() {
                collection::CollectionDestroyRequest request(name);
                invoke<bool>(request);
                context->getDistributedObjectListenerService().removeDistributedObject(name);
            };

        private:
            template<typename T>
            std::vector<serialization::Data> toDataCollection(const std::vector<T>& objects) {
                std::vector<serialization::Data> dataCollection(objects.size());
                for (int i = 0; i < objects.size(); ++i) {
                    dataCollection[i] = toData(objects[i]);
                }
                return dataCollection;
            };

            template<typename T>
            serialization::Data toData(const T& object) {
                return context->getSerializationService().toData<T>(&object);
            };

            template<typename T>
            T toObject(const serialization::Data& data) {
                return context->getSerializationService().template toObject<T>(data);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                request.setServiceName(name);
                return context->getInvocationService().template invokeOnRandomTarget<Response>(request, key);
            };

            IList() {

            };

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                context = clientContext;
                key = toData(instanceName);
                name = instanceName;
            };


            std::string name;
            spi::ClientContext *context;
            serialization::Data key;
        };
    }
}

#endif /* HAZELCAST_ILIST */