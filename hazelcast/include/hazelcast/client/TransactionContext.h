/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/TransactionalMap.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/TransactionalQueue.h"
#include "hazelcast/client/TransactionalMultiMap.h"
#include "hazelcast/client/TransactionalList.h"
#include "hazelcast/client/TransactionalSet.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace std {
    template <>
    class HAZELCAST_API hash<std::pair<std::string, std::string>> {
    public:
        std::size_t operator()(const std::pair<std::string, std::string> &val) const noexcept;
    };
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                class ClientTransactionManagerServiceImpl;
            }
        }

        namespace connection {
            class ClientConnectionManagerImpl;

            class Connection;
        }

        /**
         * Provides a context to do transactional operations; so beginning/committing transactions, but also retrieving
         * transactional data-structures like the TransactionalMap.
         *
         * @see HazelcastClient::newTransactionContext
         */
        class HAZELCAST_API TransactionContext {
        public:
            /**
             *  Constructor to be used internally. Not public API.
             *
             */
            TransactionContext(spi::impl::ClientTransactionManagerServiceImpl &transactionManager,
                               const TransactionOptions &);

            /**
             *  @return txn id.
             */
            boost::uuids::uuid  getTxnId() const;

            /**
             * Begins a transaction.
             *
             * @throws IllegalStateException if a transaction already is active.
             */
            boost::future<void> beginTransaction();

            /**
             * Commits a transaction.
             *
             * @throws TransactionException if no transaction is active or the transaction could not be committed.
             */
            boost::future<void> commitTransaction();

            /**
             * Begins a transaction.
             *
             * @throws IllegalStateException if a transaction already is active.
             */
            boost::future<void> rollbackTransaction();

            /**
             * Returns the transactional distributed map instance with the specified name.
             *
             *
             * @param name name of the distributed map
             * @return transactional distributed map instance with the specified name
            */
            std::shared_ptr<TransactionalMap> getMap(const std::string &name) {
                return getTransactionalObject<TransactionalMap>(IMap::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional queue instance with the specified name.
             *
             *
             * @param name name of the queue
             * @return transactional queue instance with the specified name
             */
            std::shared_ptr<TransactionalQueue> getQueue(const std::string &name) {
                return getTransactionalObject<TransactionalQueue>(IQueue::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional multimap instance with the specified name.
             *
             *
             * @param name name of the multimap
             * @return transactional multimap instance with the specified name
             */
            std::shared_ptr<TransactionalMultiMap> getMultiMap(const std::string &name) {
                return getTransactionalObject<TransactionalMultiMap>(MultiMap::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional list instance with the specified name.
             *
             *
             * @param name name of the list
             * @return transactional list instance with the specified name
             */
            std::shared_ptr<TransactionalList> getList(const std::string &name) {
                return getTransactionalObject<TransactionalList>(IList::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional set instance with the specified name.
             *
             *
             * @param name name of the set
             * @return transactional set instance with the specified name
             */
            std::shared_ptr<TransactionalSet> getSet(const std::string &name) {
                return getTransactionalObject<TransactionalSet>(ISet::SERVICE_NAME, name);
            }

            /**
             * get any transactional object with template T.
             *
             * Mostly to be used by spi implementers of Hazelcast.
             *
             * @return transactionalObject.
             */
            template<typename T>
            std::shared_ptr<T> getTransactionalObject(const std::string &serviceName, const std::string &name) {
                if (transaction.getState() != txn::TxnState::ACTIVE) {
                    std::string message = "No transaction is found while accessing ";
                    message += "transactional object -> [" + name + "]!";
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalStateException("TransactionContext::getMap(const std::string& name)",
                                                             message));
                }
                auto key = std::make_pair(serviceName, name);
                std::shared_ptr<T> obj = std::static_pointer_cast<T>(txnObjectMap.get(key));
                if (!obj) {
                    obj = std::shared_ptr<T>(new T(name, transaction));
                    txnObjectMap.put(key, obj);
                }

                return obj;
            }

        private :
            TransactionOptions options;
            std::shared_ptr<connection::Connection> txnConnection;
            txn::TransactionProxy transaction;
            util::SynchronizedMap<std::pair<std::string, std::string>, proxy::TransactionalObject> txnObjectMap;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

