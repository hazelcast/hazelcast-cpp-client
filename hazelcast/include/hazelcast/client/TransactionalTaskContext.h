//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalTaskContext
#define HAZELCAST_TransactionalTaskContext

#include "hazelcast/client/TransactionContext.h"

namespace hazelcast {
    namespace client {
        /**
         * Provides a context to access transactional data-structures like the TransactionalMap.
         *
         * @see HazelcastClient#executeTransaction(...)
         */
        class HAZELCAST_API TransactionalTaskContext {
        public:
            /**
             *  Constructor to be used internally. Not public API.
             *
             */
            TransactionalTaskContext(TransactionContext& context);

            /**
             * Returns the transactional distributed map instance with the specified name.
             *
             *
             * @param name name of the distributed map
             * @return transactional distributed map instance with the specified name
            */
            template<typename K, typename V>
            TransactionalMap<K, V> getMap(const std::string& name) {
                return context.getMap<K, V>(name);
            }

            /**
             * Returns the transactional queue instance with the specified name.
             *
             *
             * @param name name of the queue
             * @return transactional queue instance with the specified name
             */
            template<typename E>
            TransactionalQueue< E > getQueue(const std::string& name) {
                return context.getQueue< E >(name);
            }

            /**
             * Returns the transactional multimap instance with the specified name.
             *
             *
             * @param name name of the multimap
             * @return transactional multimap instance with the specified name
             */
            template<typename K, typename V>
            TransactionalMultiMap<K, V> getMultiMap(const std::string& name) {
                return context.getMultiMap<K, V>(name);
            }

            /**
             * Returns the transactional list instance with the specified name.
             *
             *
             * @param name name of the list
             * @return transactional list instance with the specified name
             */
            template<typename E>
            TransactionalList< E > getList(const std::string& name) {
                return context.getList< E >(name);
            }

            /**
             * Returns the transactional set instance with the specified name.
             *
             *
             * @param name name of the set
             * @return transactional set instance with the specified name
             */
            template<typename E>
            TransactionalSet< E > getSet(const std::string& name) {
                return context.getSet< E >(name);
            }

            /**
             * get any transactional object with template T.
             *
             * Mostly to be used by spi implementers of Hazelcast.
             *
             * @return transactionalObject.
             */
            template<typename T>
            T getTransactionalObject(const std::string& name) {
                return context.getTransactionalObject< T >(name);
            }

        private:
            TransactionContext& context;
        };
    }
}

#endif //HAZELCAST_TransactionalTaskContext
