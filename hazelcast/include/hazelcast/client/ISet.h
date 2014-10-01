#ifndef HAZELCAST_ISET
#define HAZELCAST_ISET

#include "hazelcast/client/proxy/ISetImpl.h"
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
        * @tparam E item type
        */
        template<typename E>
        class HAZELCAST_API ISet : public proxy::ISetImpl {
            friend class HazelcastClient;

        public:
            /**
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            *  @param listener to be added
            *  @param includeValue boolean value representing value should be included in incoming ItemEvent or not.
            *  @returns registrationId that can be used to remove item listener
            */
            std::string addItemListener(ItemListener<E>& listener, bool includeValue) {
                impl::ItemEventHandler<E> *itemEventHandler = new impl::ItemEventHandler<E>(getName(), context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return proxy::ISetImpl::addItemListener(itemEventHandler, includeValue);
            }

            /**
            * Removes the specified item listener.
            * Returns false if the specified listener is not added before.
            *
            * @param registrationId Id of listener registration.
            *
            * @return true if registration is removed, false otherwise
            */
            bool removeItemListener(const std::string& registrationId) {
                return proxy::ISetImpl::removeItemListener(registrationId);
            }

            /**
            *
            * @returns size of the distributed set
            */
            int size() {
                return proxy::ISetImpl::size();
            }

            /**
            *
            * @returns true if empty
            */
            bool isEmpty() {
                return size() == 0;
            }

            /**
            *
            * @param element to be searched
            * @returns true if set contains element
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool contains(const E& element) {
                return proxy::ISetImpl::contains(toData(element));
            }

            /**
            *
            * @returns all elements as std::vector
            */
            std::vector<E> toArray() {
                std::vector<serialization::pimpl::Data *> collection = proxy::ISetImpl::toArray();
                std::vector<E> set(collection.size());
                for (int i = 0; i < collection.size(); ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*(collection[i]));
                    set[i] = *e;
                }
                return set;
            }

            /**
            *
            * @param element to be added
            * @return true if element is added successfully. If elements was already there returns false.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool add(const E& element) {
                return proxy::ISetImpl::add(toData(element));
            }

            /**
            *
            * @param element to be removed
            * @return true if element is removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool remove(const E& element) {
                return proxy::ISetImpl::remove(toData(element));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this set contains all elements given in vector.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                return proxy::ISetImpl::containsAll(dataCollection);
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to set.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool addAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                return proxy::ISetImpl::addAll(dataCollection);
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool removeAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                return proxy::ISetImpl::removeAll(dataCollection);
            }

            /**
            *
            * Removes the elements from this set that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool retainAll(const std::vector<E>& elements) {
                return proxy::ISetImpl::retainAll(toDataCollection(elements));
            }

            /**
            *
            * Removes all elements from set.
            */
            void clear() {
                proxy::ISetImpl::clear();
            }

        private:
            ISet(const std::string& instanceName, spi::ClientContext *context)
            : proxy::ISetImpl(instanceName, context) {
            }
        };
    }
}

#endif /* HAZELCAST_ISET */

