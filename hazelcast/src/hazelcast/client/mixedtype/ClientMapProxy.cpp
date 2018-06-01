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

#include "hazelcast/client/mixedtype/ClientMapProxy.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            ClientMapProxy::ClientMapProxy(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IMapImpl(instanceName, context) {
            }

            void ClientMapProxy::removeAll(const query::Predicate &predicate) {
                serialization::pimpl::Data predicateData = toData(predicate);

                removeAllInternal(predicateData);
            }

            void ClientMapProxy::flush() {
                proxy::IMapImpl::flush();
            }

            void ClientMapProxy::removeInterceptor(const std::string &id) {
                proxy::IMapImpl::removeInterceptor(id);
            }

            std::string ClientMapProxy::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), context->getClientClusterService(), context->getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            std::string
            ClientMapProxy::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                       bool includeValue) {
                impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                getName(), context->getClientClusterService(), context->getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, predicate, includeValue);
            }

            bool ClientMapProxy::removeEntryListener(const std::string &registrationId) {
                return proxy::IMapImpl::removeEntryListener(registrationId);
            }

            void ClientMapProxy::evictAll() {
                proxy::IMapImpl::evictAll();
            }

            std::vector<TypedData> ClientMapProxy::keySet() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData();
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, context->getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> ClientMapProxy::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return keySet(*p);
            }

            std::vector<TypedData> ClientMapProxy::keySet(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, context->getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> ClientMapProxy::values() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData();
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> valueData(new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, context->getSerializationService());
                }
                return values;
            }

            std::vector<TypedData> ClientMapProxy::values(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return values(*p);
            }

            std::vector<TypedData> ClientMapProxy::values(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> valueData(new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, context->getSerializationService());
                }
                return values;
            }

            std::vector<std::pair<TypedData, TypedData> > ClientMapProxy::entrySet() {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData();
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::auto_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = context->getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService), TypedData(valueData,
                                                                                                    serializationService));
                }
                return entries;
            }

            std::vector<std::pair<TypedData, TypedData> >
            ClientMapProxy::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                        predicate);
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::auto_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = context->getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService),
                                                TypedData(valueData, serializationService));
                }
                return entries;
            }

            std::vector<std::pair<TypedData, TypedData> > ClientMapProxy::entrySet(const query::Predicate &predicate) {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                        predicate);
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::auto_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = context->getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService),
                                                TypedData(valueData, serializationService));
                }
                return entries;
            }

            void ClientMapProxy::addIndex(const std::string &attribute, bool ordered) {
                proxy::IMapImpl::addIndex(attribute, ordered);
            }

            int ClientMapProxy::size() {
                return proxy::IMapImpl::size();
            }

            bool ClientMapProxy::isEmpty() {
                return proxy::IMapImpl::isEmpty();
            }

            void ClientMapProxy::clear() {
                proxy::IMapImpl::clear();
            }

            serialization::pimpl::SerializationService &ClientMapProxy::getSerializationService() const {
                return context->getSerializationService();
            }

            monitor::LocalMapStats &ClientMapProxy::getLocalMapStats() {
                return stats;
            }

            boost::shared_ptr<TypedData> ClientMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                std::auto_ptr<serialization::pimpl::Data> valueData = proxy::IMapImpl::getData(keyData);
                return boost::shared_ptr<TypedData>(new TypedData(valueData, context->getSerializationService()));
            }

            bool ClientMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::containsKey(keyData);
            }

            std::auto_ptr<serialization::pimpl::Data> ClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::removeData(keyData);
            }

            bool ClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::remove(keyData, valueData);
            }

            void ClientMapProxy::removeAllInternal(const serialization::pimpl::Data &predicateData) {
                return proxy::IMapImpl::removeAll(predicateData);
            }

            void ClientMapProxy::deleteInternal(const serialization::pimpl::Data &keyData) {
                proxy::IMapImpl::deleteEntry(keyData);
            }

            bool
            ClientMapProxy::tryRemoveInternal(const serialization::pimpl::Data &keyData, long timeoutInMillis) {
                return proxy::IMapImpl::tryRemove(keyData, timeoutInMillis);
            }

            bool ClientMapProxy::tryPutInternal(const serialization::pimpl::Data &keyData,
                                          const serialization::pimpl::Data &valueData,
                                          long timeoutInMillis) {
                return proxy::IMapImpl::tryPut(keyData, valueData, timeoutInMillis);
            }

            std::auto_ptr<serialization::pimpl::Data>
            ClientMapProxy::putInternal(const serialization::pimpl::Data &keyData,
                                  const serialization::pimpl::Data &valueData, long timeoutInMillis) {
                return proxy::IMapImpl::putData(keyData, valueData, timeoutInMillis);
            }

            void ClientMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                                   const serialization::pimpl::Data &valueData,
                                                   int ttlInMillis) {
                proxy::IMapImpl::tryPut(keyData, valueData, ttlInMillis);
            }

            std::auto_ptr<serialization::pimpl::Data>
            ClientMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                          const serialization::pimpl::Data &valueData,
                                          int ttlInMillis) {
                return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttlInMillis);
            }

            bool ClientMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                 const serialization::pimpl::Data &valueData,
                                                 const serialization::pimpl::Data &newValueData) {
                return proxy::IMapImpl::replace(keyData, valueData, newValueData);
            }

            std::auto_ptr<serialization::pimpl::Data>
            ClientMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                      const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::replaceData(keyData, valueData);

            }

            void ClientMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                       const serialization::pimpl::Data &valueData,
                                       int ttlInMillis) {
                proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
            }

            bool ClientMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::evict(keyData);
            }

            EntryVector ClientMapProxy::getAllInternal(const PID_TO_KEY_MAP &partitionToKeyData) {
                std::map<int, std::vector<serialization::pimpl::Data> > datas;
                for (PID_TO_KEY_MAP::const_iterator it = partitionToKeyData.begin();
                     it != partitionToKeyData.end(); ++it) {
                    const std::vector<boost::shared_ptr<serialization::pimpl::Data> > &valueDatas = it->second;
                    for (std::vector<boost::shared_ptr<serialization::pimpl::Data> >::const_iterator valueIt = valueDatas.begin();
                         valueIt != valueDatas.end(); ++valueIt) {
                        datas[it->first].push_back(*(*valueIt));
                    }
                }
                return proxy::IMapImpl::getAllData(datas);
            }

            std::auto_ptr<serialization::pimpl::Data>
            ClientMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                           const serialization::pimpl::Data &processor) {
                return proxy::IMapImpl::executeOnKeyData(keyData, processor);
            }

            std::auto_ptr<serialization::pimpl::Data>
            ClientMapProxy::submitToKeyDecoder(protocol::ClientMessage &response) {
                return protocol::codec::MapExecuteOnKeyCodec::ResponseParameters::decode(response).response;
            }

            void
            ClientMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                proxy::IMapImpl::putAllData(entries);
            }

        }
    }
}
