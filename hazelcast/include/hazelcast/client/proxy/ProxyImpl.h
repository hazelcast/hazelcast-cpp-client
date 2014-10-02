//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_ProxyImpl
#define HAZELCAST_ProxyImpl

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include <string>

#define DESERIALIZE(data, RETURN_TYPE) boost::shared_ptr<RETURN_TYPE> result = context->getSerializationService().toObject<RETURN_TYPE>(data);

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace impl {
            class BaseEventHandler;

            class BaseRemoveListenerRequest;

            class ClientRequest;
        }

        namespace spi {
            class ClientContext;
        }

        namespace proxy {
            class HAZELCAST_API ProxyImpl : public DistributedObject{
            protected:
                /**
                * Constructor
                */
                ProxyImpl(const std::string& serviceName, const std::string& objectName, spi::ClientContext *context);

                /**
                * Destructor
                */
                virtual ~ProxyImpl();

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param partitionId that given request will be send to.
                * @param request ClientRequest ptr.
                */
                serialization::pimpl::Data invoke(const impl::ClientRequest *request, int partitionId);

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param request ClientRequest ptr.
                */
                serialization::pimpl::Data invoke(const impl::ClientRequest *request);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientRequest ptr.
                * @param partitionId
                * @param handler
                */
                std::string listen(const impl::ClientRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientRequest ptr.
                * @param handler
                */
                std::string listen(const impl::ClientRequest *registrationRequest, impl::BaseEventHandler *handler);

                /**
                * Internal API.
                *
                * @param request ClientRequest ptr.
                * @param registrationId
                */
                bool stopListening(impl::BaseRemoveListenerRequest *request, const std::string& registrationId);

                /**
                * Internal API.
                * @param key
                */
                int getPartitionId(const serialization::pimpl::Data& key);

                template<typename T>
                serialization::pimpl::Data toData(const T& object) {
                    return context->getSerializationService().template toData<T>(&object);
                }

                template<typename T>
                boost::shared_ptr<T> toObject(const serialization::pimpl::Data& data) {
                    return context->getSerializationService().template toObject<T>(data);
                }

                template <typename V>
                std::vector<V> toObjectCollection(std::vector<serialization::pimpl::Data> const& dataCollection) {
                    size_t size = dataCollection.size();
                    std::vector<V> multimap(size);
                    for (int i = 0; i < size; i++) {
                        boost::shared_ptr<V> v = toObject<V>(dataCollection[i]);
                        multimap[i] = *v;
                    }
                    return multimap;
                }

                template<typename T>
                const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T>& elements) {
                    size_t size = elements.size();
                    std::vector<serialization::pimpl::Data> dataCollection(size);
                    for (int i = 0; i < size; ++i) {
                        dataCollection[i] = toData(elements[i]);
                    }
                    return dataCollection;
                }

                template <typename K, typename V>
                std::vector<std::pair<K, V> > toObjectEntrySet(std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > const& dataEntrySet) {
                    size_t size = dataEntrySet.size();
                    std::vector<std::pair<K, V> > entrySet(size);
                    for (int i = 0; i < size; i++) {
                        boost::shared_ptr<K> key = toObject<K>(dataEntrySet[i].first);
                        entrySet[i].first = *key;
                        boost::shared_ptr<V> value = toObject<V>(dataEntrySet[i].second);
                        entrySet[i].second = *value;
                    }
                    return entrySet;
                }

                template <typename K, typename V>
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > toDataEntriesSet(std::map<K, V> const& m) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entryDataSet(m.size());
                    typename std::map<K,V>::const_iterator it;
                    int i = 0;
                    for (it = m.begin(); it != m.end(); ++it) {
                        entryDataSet[i++] = std::make_pair(toData(it->first), toData(it->second));
                    }
                    return entryDataSet;
                }

                spi::ClientContext *context;
            public:
                /**
                * Destroys this object cluster-wide.
                * Clears and releases all resources for this object.
                */
                void destroy();
            };
        }
    }
}

#endif //HAZELCAST_ProxyImpl
