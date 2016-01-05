/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/IRemoveListenerCodec.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class DataEntryView;
        }
        namespace connection {
            class Connection;
        }

        namespace impl {
            class BaseEventHandler;

            class BaseRemoveListenerRequest;

            class ClientRequest;
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

        typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > EntryVector;

        namespace proxy {
            class HAZELCAST_API ProxyImpl : public DistributedObject {
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
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param partitionId that given request will be send to.
                * @param request ClientRequest ptr.
                */
                std::auto_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                              int partitionId);

                connection::CallFuture invokeAndGetFuture(std::auto_ptr<protocol::ClientMessage> request,
                                                              int partitionId);

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param request ClientMessage ptr.
                */
                std::auto_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request);

                std::auto_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                              boost::shared_ptr<connection::Connection> conn);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientRequest ptr.
                * @param partitionId
                * @param handler
                */
                std::string registerListener(std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                                             int partitionId, impl::BaseEventHandler *handler);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientRequest ptr.
                * @param handler
                */
                std::string registerListener(std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
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
                boost::shared_ptr<T> toObject(const serialization::pimpl::Data &data) {
                    return context->getSerializationService().template toObject<T>(data);
                }

                template<typename T>
                boost::shared_ptr<T> toObject(std::auto_ptr<serialization::pimpl::Data> data) {
                    if (NULL == data.get()) {
                        return boost::shared_ptr<T>();
                    } else {
                        return toObject<T>(*data);
                    }
                }

                template<typename V>
                std::vector<V> toObjectCollection(const std::vector<serialization::pimpl::Data> &collection) {
                    size_t size = collection.size();
                    std::vector<V> multimap(size);
                    for (size_t i = 0; i < size; i++) {
                        boost::shared_ptr<V> v = toObject<V>(collection[i]);
                        multimap[i] = *v;
                    }
                    return multimap;
                }

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
                        boost::shared_ptr<K> key = toObject<K>(dataEntrySet[i].first);
                        entrySet[i].first = *key;
                        boost::shared_ptr<V> value = toObject<V>(dataEntrySet[i].second);
                        entrySet[i].second = *value;
                    }
                    return entrySet;
                }

                template<typename K, typename V>
                EntryVector toDataEntriesSet(std::map<K, V> const &m) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entryDataSet(
                            m.size());
                    typename std::map<K, V>::const_iterator it;
                    int i = 0;
                    for (it = m.begin(); it != m.end(); ++it) {
                        entryDataSet[i++] = std::make_pair(toData(it->first), toData(it->second));
                    }
                    return entryDataSet;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     boost::shared_ptr<connection::Connection> conn) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, conn);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, partitionId);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetFuture(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {

                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, partitionId);

                    return (T)CODEC::decode(*response).response;
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     int partitionId, boost::shared_ptr<connection::Connection> conn) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, conn);

                    return (T)CODEC::decode(*response).response;
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
