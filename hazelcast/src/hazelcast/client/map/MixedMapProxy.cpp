/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/map/MixedMapProxy.h"

namespace hazelcast {
    namespace client {
        namespace map {
            MixedMapProxy::MixedMapProxy(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IMapImpl(instanceName, context) {
            }

            void MixedMapProxy::removeAll(const query::Predicate &predicate) {
                serialization::pimpl::Data predicateData = toData(predicate);

                removeAllInternal(predicateData);
            }

            void MixedMapProxy::flush() {
                proxy::IMapImpl::flush();
            }

            void MixedMapProxy::removeInterceptor(const std::string &id) {
                proxy::IMapImpl::removeInterceptor(id);
            }

            std::string MixedMapProxy::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                client::impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new client::impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), context->getClusterService(), context->getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            std::string
            MixedMapProxy::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                            bool includeValue) {
                client::impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler> *entryEventHandler =
                        new client::impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                getName(), context->getClusterService(), context->getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, predicate, includeValue);
            }

            bool MixedMapProxy::removeEntryListener(const std::string &registrationId) {
                return proxy::IMapImpl::removeEntryListener(registrationId);
            }

            EntryView<TypedData, TypedData> MixedMapProxy::getEntryView(const TypedData &key) {
                std::auto_ptr<map::DataEntryView> dataEntryView = proxy::IMapImpl::getEntryViewData(*key.getData());
                TypedData value(std::auto_ptr<serialization::pimpl::Data>(
                        new serialization::pimpl::Data(dataEntryView->getValue())),
                                context->getSerializationService());
                EntryView<TypedData, TypedData> view(key, value, *dataEntryView);
                return view;
            }

            void MixedMapProxy::evictAll() {
                proxy::IMapImpl::evictAll();
            }

            std::vector<TypedData> MixedMapProxy::keySet() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData();
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, context->getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> MixedMapProxy::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return keySet(*p);
            }

            std::vector<TypedData> MixedMapProxy::keySet(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, context->getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> MixedMapProxy::values() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData();
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> valueData(new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, context->getSerializationService());
                }
                return values;
            }

            std::vector<TypedData> MixedMapProxy::values(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return values(*p);
            }

            std::vector<TypedData> MixedMapProxy::values(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::auto_ptr<serialization::pimpl::Data> valueData(new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, context->getSerializationService());
                }
                return values;
            }

            std::vector<std::pair<TypedData, TypedData> > MixedMapProxy::entrySet() {
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
            MixedMapProxy::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
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

            std::vector<std::pair<TypedData, TypedData> > MixedMapProxy::entrySet(const query::Predicate &predicate) {
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

            void MixedMapProxy::addIndex(const std::string &attribute, bool ordered) {
                proxy::IMapImpl::addIndex(attribute, ordered);
            }

            int MixedMapProxy::size() {
                return proxy::IMapImpl::size();
            }

            bool MixedMapProxy::isEmpty() {
                return proxy::IMapImpl::isEmpty();
            }

            void MixedMapProxy::clear() {
                proxy::IMapImpl::clear();
            }

            serialization::pimpl::SerializationService &MixedMapProxy::getSerializationService() const {
                return context->getSerializationService();
            }

            monitor::LocalMapStats &MixedMapProxy::getLocalMapStats() {
                return stats;
            }

            TypedData MixedMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                std::auto_ptr<serialization::pimpl::Data> valueData = proxy::IMapImpl::getData(keyData);
                return TypedData(valueData, context->getSerializationService());
            }

            bool MixedMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::containsKey(keyData);
            }

            std::auto_ptr<serialization::pimpl::Data> MixedMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::removeData(keyData);
            }

            bool MixedMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::remove(keyData, valueData);
            }

            void MixedMapProxy::removeAllInternal(const serialization::pimpl::Data &predicateData) {
                return proxy::IMapImpl::removeAll(predicateData);
            }

            void MixedMapProxy::deleteInternal(const serialization::pimpl::Data &keyData) {
                proxy::IMapImpl::deleteEntry(keyData);
            }

            bool
            MixedMapProxy::tryRemoveInternal(const serialization::pimpl::Data &keyData, long timeoutInMillis) {
                return proxy::IMapImpl::tryRemove(keyData, timeoutInMillis);
            }

            bool MixedMapProxy::tryPutInternal(const serialization::pimpl::Data &keyData,
                                                       const serialization::pimpl::Data &valueData,
                                                       long timeoutInMillis) {
                return proxy::IMapImpl::tryPut(keyData, valueData, timeoutInMillis);
            }

            std::auto_ptr<serialization::pimpl::Data>
            MixedMapProxy::putInternal(const serialization::pimpl::Data &keyData,
                                       const serialization::pimpl::Data &valueData, long timeoutInMillis) {
                return proxy::IMapImpl::putData(keyData, valueData, timeoutInMillis);
            }

            void MixedMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                                                const serialization::pimpl::Data &valueData,
                                                                int ttlInMillis) {
                proxy::IMapImpl::tryPut(keyData, valueData, ttlInMillis);
            }

            std::auto_ptr<serialization::pimpl::Data>
            MixedMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                               const serialization::pimpl::Data &valueData,
                                               int ttlInMillis) {
                return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttlInMillis);
            }

            bool MixedMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                              const serialization::pimpl::Data &valueData,
                                                              const serialization::pimpl::Data &newValueData) {
                return proxy::IMapImpl::replace(keyData, valueData, newValueData);
            }

            std::auto_ptr<serialization::pimpl::Data>
            MixedMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                           const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::replaceData(keyData, valueData);

            }

            void MixedMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                                    const serialization::pimpl::Data &valueData,
                                                    int ttlInMillis) {
                proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
            }

            bool MixedMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::evict(keyData);
            }

            std::auto_ptr<serialization::pimpl::Data>
            MixedMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                                const serialization::pimpl::Data &processor) {
                return proxy::IMapImpl::executeOnKeyData(keyData, processor);
            }

            std::auto_ptr<serialization::pimpl::Data>
            MixedMapProxy::submitToKeyDecoder(protocol::ClientMessage &response) {
                return protocol::codec::MapExecuteOnKeyCodec::ResponseParameters::decode(response).response;
            }

            void
            MixedMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                proxy::IMapImpl::putAllData(entries);
            }

            boost::shared_ptr<serialization::pimpl::Data>
            MixedMapProxy::toShared(const serialization::pimpl::Data &data) {
                return boost::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(data));
            }
        }
    }
}
