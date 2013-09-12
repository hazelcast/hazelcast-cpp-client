#ifndef HAZELCAST_ISET
#define HAZELCAST_ISET

#include "hazelcast/client/spi/DistributedObjectListenerService.h"
#include "CollectionAddListenerRequest.h"
#include "CollectionSizeRequest.h"
#include "CollectionContainsRequest.h"
#include "CollectionRemoveRequest.h"
#include "CollectionAddAllRequest.h"
#include "CollectionCompareAndRemoveRequest.h"
#include "CollectionGetAllRequest.h"
#include "CollectionAddRequest.h"
#include "CollectionClearRequest.h"
#include "CollectionDestroyRequest.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        template<typename E>
        class ISet {
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
                collection::CollectionAddRequest request(name, valueData);
                return invoke<E>(request);
            };

            bool remove(const E& e) {
                serialization::Data valueData = toData(e);
                collection::CollectionRemoveRequest request(name, key, valueData, util::getThreadId());
                return invoke<bool>(request);
            };

            bool containsAll(const std::vector<E>& objects) {
                collection::CollectionContainsRequest request(name, key, toDataCollection(objects));
                return invoke<bool>(request);
            };

            bool addAll(const std::vector<E>& objects) {
                collection::CollectionAddAllRequest request(name, key, util::getThreadId(), toDataCollection(objects));
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
                collection::CollectionClearRequest request(name);
                invoke<bool>(request);
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

            ISet() {

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

#endif /* HAZELCAST_ISET */