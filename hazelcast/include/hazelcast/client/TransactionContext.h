//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionContext
#define HAZELCAST_TransactionContext

#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/TransactionalMap.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/TransactionalQueue.h"
#include "hazelcast/client/TransactionalMultiMap.h"
#include "hazelcast/client/TransactionalList.h"
#include "hazelcast/client/TransactionalSet.h"

namespace hazelcast {
    namespace client {

        namespace spi {
            class PartitionService;
        }

        namespace connection {
            class ConnectionManager;
        }

        class HAZELCAST_API TransactionContext {
        public:
            TransactionContext(spi::ClusterService &, serialization::SerializationService &, connection::ConnectionManager &, const TransactionOptions &);

            std::string getTxnId() const;

            void beginTransaction();

            void commitTransaction();

            void rollbackTransaction();

            template<typename K, typename V>
            TransactionalMap<K, V> getMap(const std::string &name) {
                return getTransactionalObject< TransactionalMap<K, V> >(name);
            }

            template<typename E>
            TransactionalQueue< E > getQueue(const std::string &name) {
                return getTransactionalObject< TransactionalQueue< E > >(name);
            }

            template<typename K, typename V>
            TransactionalMultiMap<K, V> getMultiMap(const std::string &name) {
                return getTransactionalObject< TransactionalMultiMap<K, V> >(name);
            }

            template<typename E>
            TransactionalList< E > getList(const std::string &name) {
                return getTransactionalObject< TransactionalList< E > >(name);
            }

            template<typename E>
            TransactionalSet< E > getSet(const std::string &name) {
                return getTransactionalObject< TransactionalSet< E > >(name);
            }

            template<typename T>
            T getTransactionalObject(const std::string &name) {
                if (transaction.getState() != txn::TxnState::ACTIVE) {
                    std::string message = "No transaction is found while accessing ";
                    message += "transactional object -> [" + name + "]!";
                    throw exception::IllegalStateException("TransactionContext::getMap(const std::string& name)", message);
                }
                T txnObject(name, &transaction);
                return txnObject;
            }

        private :
            const int CONNECTION_TRY_COUNT;
            connection::ConnectionManager &connectionManager;
            TransactionOptions options;
            connection::Connection *txnConnection;
            txn::TransactionProxy transaction;

            connection::Connection *connect();

        };

    }
}

#endif //HAZELCAST_TransactionContext
