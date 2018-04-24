/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 01/10/14.
//
#ifndef HAZELCAST_ProxyImpl
#define HAZELCAST_ProxyImpl

#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/spi/EventHandler.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }

        namespace impl {
            class BaseEventHandler;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class IAddListenerCodec;
            }
        }

        namespace spi {
            namespace impl {
                class ClientInvocationFuture;
            }
        }

        typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > EntryVector;

        namespace proxy {
            class HAZELCAST_API ProxyImpl : public DistributedObject, public spi::ClientProxy {
            public:
                /**
                * Destroys this object cluster-wide.
                * Clears and releases all resources for this object.
                */
                virtual void destroy();

                template <typename T>
                TypedData toTypedData(const T &key) {
                    return TypedData(toHeapData<T>(key), context->getSerializationService());
                }

            protected:
                /**
                * Constructor
                */
                ProxyImpl(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context);

                /**
                * Destructor
                */
                virtual ~ProxyImpl();

                /**
                 * Called when proxy is created.
                 * Overriding implementations can add initialization specific logic into this method
                 * like registering a listener, creating a cleanup task etc.
                 */
                virtual void onInitialize() {
                }

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param partitionId that given request will be send to.
                * @param request Client request message to be sent.
                */
                boost::shared_ptr<protocol::ClientMessage> invokeOnPartition(std::auto_ptr<protocol::ClientMessage> request,
                                                                         int partitionId);

                boost::shared_ptr<spi::impl::ClientInvocationFuture> invokeAndGetFuture(std::auto_ptr<protocol::ClientMessage> request,
                                                              int partitionId);

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param request ClientMessage ptr.
                */
                boost::shared_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request);

                boost::shared_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                              boost::shared_ptr<connection::Connection> conn);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientMessage ptr.
                * @param handler
                */
                std::string registerListener(const boost::shared_ptr<spi::impl::ListenerMessageCodec> &codec,
                                             impl::BaseEventHandler *handler);

                /**
                * Internal API.
                * @param key
                */
                int getPartitionId(const serialization::pimpl::Data &key);

                template<typename T>
                serialization::pimpl::Data toData(const T &object) {
                    return context->getSerializationService().template toData<T>(&object);
                }

                template<typename T>
                std::auto_ptr<T> toObject(const serialization::pimpl::Data &data) {
                    return context->getSerializationService().template toObject<T>(data);
                }

                template<typename T>
                std::auto_ptr<T> toObject(std::auto_ptr<serialization::pimpl::Data> data) {
                    if (NULL == data.get()) {
                        return std::auto_ptr<T>();
                    } else {
                        return toObject<T>(*data);
                    }
                }

                template<typename V>
                std::vector<V> toObjectCollection(const std::vector<serialization::pimpl::Data> &collection) {
                    size_t size = collection.size();
                    std::vector<V> objectArray(size);
                    for (size_t i = 0; i < size; i++) {
                        std::auto_ptr<V> v = toObject<V>(collection[i]);
                        objectArray[i] = *v;
                    }
                    return objectArray;
                }

                std::vector<TypedData>
                toTypedDataCollection(const std::vector<serialization::pimpl::Data> &values) const;

                template<typename T>
                const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T> &elements) {
                    size_t size = elements.size();
                    std::vector<serialization::pimpl::Data> dataCollection(size);
                    for (size_t i = 0; i < size; ++i) {
                        dataCollection[i] = toData(elements[i]);
                    }
                    return dataCollection;
                }

                template<typename K, typename V>
                std::vector<std::pair<K, V> > toObjectEntrySet(
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > const &dataEntrySet) {
                    size_t size = dataEntrySet.size();
                    std::vector<std::pair<K, V> > entrySet(size);
                    for (size_t i = 0; i < size; i++) {
                        std::auto_ptr<K> key = toObject<K>(dataEntrySet[i].first);
                        entrySet[i].first = *key;
                        std::auto_ptr<V> value = toObject<V>(dataEntrySet[i].second);
                        entrySet[i].second = *value;
                    }
                    return entrySet;
                }

                std::vector<std::pair<TypedData, TypedData> > toTypedDataEntrySet(
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &dataEntrySet);

                template<typename K, typename V>
                EntryVector toDataEntries(std::map<K, V> const &m) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entries(
                            m.size());
                    int i = 0;
                    for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
                        entries[i++] = std::make_pair(toData(it->first), toData(it->second));
                    }
                    return entries;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request) {
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(request);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     boost::shared_ptr<connection::Connection> conn) {
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(request, conn);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                    boost::shared_ptr<protocol::ClientMessage> response = invokeOnPartition(request, partitionId);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     int partitionId, boost::shared_ptr<connection::Connection> conn) {
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(request, conn);

                    return (T)CODEC::decode(*response).response;
                }


                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     const serialization::pimpl::Data &key) {
                    int partitionId = getPartitionId(key);
                    
                    boost::shared_ptr<protocol::ClientMessage> response = invokeOnPartition(request, partitionId);

                    return (T)CODEC::decode(*response).response;
                }

                boost::shared_ptr<serialization::pimpl::Data> toShared(const serialization::pimpl::Data &data);

                spi::ClientContext *context;
            private:
                class EventHandlerDelegator : public spi::EventHandler<protocol::ClientMessage> {
                public:
                    EventHandlerDelegator(impl::BaseEventHandler *handler);

                    virtual void handle(const boost::shared_ptr<protocol::ClientMessage> &event);

                    virtual void beforeListenerRegister();

                    virtual void onListenerRegister();

                private:
                    impl::BaseEventHandler *handler;
                };

                template <typename T>
                std::auto_ptr<serialization::pimpl::Data> toHeapData(const T *key) {
                    serialization::pimpl::Data data = context->getSerializationService().toData<T>(&key);
                    return std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(data));
                }
            };
        }
    }
}

#endif //HAZELCAST_ProxyImpl
