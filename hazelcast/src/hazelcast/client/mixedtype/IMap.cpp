/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/mixedtype/IMap.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            IMap::IMap(std::shared_ptr<mixedtype::ClientMapProxy> proxy) : mapImpl(proxy) {
            }

            void IMap::removeAll(const query::Predicate &predicate) {
                mapImpl->removeAll(predicate);
            }

            void IMap::flush() {
                mapImpl->flush();
            }

            void IMap::removeInterceptor(const std::string &id) {
                mapImpl->removeInterceptor(id);
            }

            std::string IMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                return mapImpl->addEntryListener(listener, includeValue);
            }

            std::string
            IMap::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                       bool includeValue) {
                return mapImpl->addEntryListener(listener, predicate, includeValue);
            }

            bool IMap::removeEntryListener(const std::string &registrationId) {
                return mapImpl->removeEntryListener(registrationId);
            }

            void IMap::evictAll() {
                mapImpl->evictAll();
            }

            std::vector<TypedData> IMap::keySet() {
                return mapImpl->keySet();
            }

            std::vector<TypedData> IMap::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::keySet(const query::Predicate &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::values() {
                return mapImpl->values();
            }

            std::vector<TypedData> IMap::values(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<TypedData> IMap::values(const query::Predicate &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet() {
                return mapImpl->entrySet();
            }

            std::vector<std::pair<TypedData, TypedData> >
            IMap::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->entrySet(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet(const query::Predicate &predicate) {
                return mapImpl->entrySet(predicate);
            }

            void IMap::addIndex(const std::string &attribute, bool ordered) {
                mapImpl->addIndex(attribute, ordered);
            }

            int IMap::size() {
                return mapImpl->size();
            }

            bool IMap::isEmpty() {
                return mapImpl->isEmpty();
            }

            void IMap::clear() {
                return mapImpl->clear();
            }

            void IMap::destroy() {
                mapImpl->destroy();
            }

            monitor::LocalMapStats &IMap::getLocalMapStats() {
                return mapImpl->getLocalMapStats();
            }
        }
    }
}
