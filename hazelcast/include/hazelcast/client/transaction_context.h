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

#include "hazelcast/client/transaction_options.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/transactional_map.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/transactional_queue.h"
#include "hazelcast/client/transactional_multi_map.h"
#include "hazelcast/client/transactional_list.h"
#include "hazelcast/client/transactional_set.h"

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
         * @see hazelcast_client::newTransactionContext
         */
        class HAZELCAST_API transaction_context {
        public:
            /**
             *  Constructor to be used internally. Not public API.
             *
             */
            transaction_context(spi::impl::ClientTransactionManagerServiceImpl &transaction_manager,
                                const transaction_options &);

            /**
             *  @return txn id.
             */
            boost::uuids::uuid get_txn_id() const;

            /**
             * Begins a transaction.
             *
             * @throws illegal_state if a transaction already is active.
             */
            boost::future<void> begin_transaction();

            /**
             * Commits a transaction.
             *
             * @throws transaction if no transaction is active or the transaction could not be committed.
             */
            boost::future<void> commit_transaction();

            /**
             * Begins a transaction.
             *
             * @throws illegal_state if a transaction already is active.
             */
            boost::future<void> rollback_transaction();

            /**
             * Returns the transactional distributed map instance with the specified name.
             *
             *
             * @param name name of the distributed map
             * @return transactional distributed map instance with the specified name
            */
            std::shared_ptr<transactional_map> get_map(const std::string &name) {
                return get_transactional_object<transactional_map>(imap::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional queue instance with the specified name.
             *
             *
             * @param name name of the queue
             * @return transactional queue instance with the specified name
             */
            std::shared_ptr<transactional_queue> get_queue(const std::string &name) {
                return get_transactional_object<transactional_queue>(iqueue::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional multimap instance with the specified name.
             *
             *
             * @param name name of the multimap
             * @return transactional multimap instance with the specified name
             */
            std::shared_ptr<transactional_multi_map> get_multi_map(const std::string &name) {
                return get_transactional_object<transactional_multi_map>(multi_map::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional list instance with the specified name.
             *
             *
             * @param name name of the list
             * @return transactional list instance with the specified name
             */
            std::shared_ptr<transactional_list> get_list(const std::string &name) {
                return get_transactional_object<transactional_list>(ilist::SERVICE_NAME, name);
            }

            /**
             * Returns the transactional set instance with the specified name.
             *
             *
             * @param name name of the set
             * @return transactional set instance with the specified name
             */
            std::shared_ptr<transactional_set> get_set(const std::string &name) {
                return get_transactional_object<transactional_set>(iset::SERVICE_NAME, name);
            }

            /**
             * get any transactional object with template T.
             *
             * Mostly to be used by spi implementers of Hazelcast.
             *
             * @return transactionalObject.
             */
            template<typename T>
            std::shared_ptr<T> get_transactional_object(const std::string &service_name, const std::string &name) {
                if (transaction_.get_state() != txn::TxnState::ACTIVE) {
                    std::string message = "No transaction is found while accessing ";
                    message += "transactional object -> [" + name + "]!";
                    BOOST_THROW_EXCEPTION(
                            exception::illegal_state("TransactionContext::getMap(const std::string& name)",
                                                             message));
                }
                auto key = std::make_pair(service_name, name);
                std::shared_ptr<T> obj = std::static_pointer_cast<T>(txn_object_map_.get(key));
                if (!obj) {
                    obj = std::shared_ptr<T>(new T(name, transaction_));
                    txn_object_map_.put(key, obj);
                }

                return obj;
            }

        private :
            transaction_options options_;
            std::shared_ptr<connection::Connection> txn_connection_;
            txn::TransactionProxy transaction_;
            util::SynchronizedMap<std::pair<std::string, std::string>, proxy::TransactionalObject> txn_object_map_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

