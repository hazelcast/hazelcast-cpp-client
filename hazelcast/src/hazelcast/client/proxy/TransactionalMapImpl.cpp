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

#include "hazelcast/client/proxy/TransactionalMapImpl.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/map/TxnMapRequest.h"
#include "hazelcast/client/impl/MapKeySet.h"
#include "hazelcast/client/impl/MapValueCollection.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalMapImpl::TransactionalMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:mapService", name, transactionProxy) {

            }

            bool TransactionalMapImpl::containsKey(const serialization::pimpl::Data& key) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::CONTAINS_KEY, key);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            serialization::pimpl::Data TransactionalMapImpl::get(const serialization::pimpl::Data& key) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::GET, key);
                return invoke(request);
            }

            int TransactionalMapImpl::size() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::SIZE);
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }

            serialization::pimpl::Data TransactionalMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::PUT, key, value);
                return invoke(request);
            }

            void TransactionalMapImpl::set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::SET, key, value);
                invoke(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::putIfAbsent(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::PUT_IF_ABSENT, key, value);
                return invoke(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REPLACE, key, value);
                return invoke(request);
            }

            bool TransactionalMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REPLACE, key, oldValue, newValue);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            serialization::pimpl::Data TransactionalMapImpl::remove(const serialization::pimpl::Data& key) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REMOVE, key);
                return invoke(request);
            }

            void TransactionalMapImpl::deleteEntry(const serialization::pimpl::Data& key) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::DELETE_R, key);
                invoke(request);
            }

            bool TransactionalMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REMOVE, key, value);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::keySet() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::KEYSET);
                boost::shared_ptr<map::MapKeySet> result = toObject<map::MapKeySet>(invoke(request));
                return result->getKeySet();
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::keySet(const std::string& predicate) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::KEYSET_BY_PREDICATE, predicate);
                boost::shared_ptr<map::MapKeySet> result = toObject<map::MapKeySet>(invoke(request));
                return result->getKeySet();
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::values() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::VALUES);
                boost::shared_ptr<map::MapValueCollection> result = toObject<map::MapValueCollection>(invoke(request));
                return result->getValues();
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::values(const std::string& predicate) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::VALUES_BY_PREDICATE, predicate);
                boost::shared_ptr<map::MapValueCollection> result = toObject<map::MapValueCollection>(invoke(request));
                return result->getValues();
            }
        }
    }
}

