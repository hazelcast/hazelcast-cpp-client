//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalList
#define HAZELCAST_TransactionalList

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/TxnListAddRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/collection/TxnListRemoveRequest.h"
#include "hazelcast/client/collection/TxnListSizeRequest.h"
#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {

        /**
         * Transactional implementation of IList.
         */
        template <typename E>
        class HAZELCAST_API TransactionalList : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            /**
             * Add new item to transactional list
             * @param e item
             * @return true if item is added successfully
             */
            bool add(const E &e) {
                serialization::pimpl::Data data = toData(e);
                collection::TxnListAddRequest *request = new collection::TxnListAddRequest(getName(), data);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return success;
            }

            /**
             * Add item from transactional list
             * @param e item
             * @return true if item is remove successfully
             */
            bool remove(const E &e) {
                serialization::pimpl::Data data = toData(e);
                collection::TxnListRemoveRequest *request = new collection::TxnListRemoveRequest(getName(), data);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            /**
             * Returns the size of the list
             * @return size
             */
            int size() {
                collection::TxnListSizeRequest *request = new collection::TxnListSizeRequest(getName());
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }

        private:
            TransactionalList(const std::string &instanceName, txn::TransactionProxy *context)
            :TransactionalObject("hz:impl:listService", instanceName, context) {

            }

            void onDestroy() {
            }

        };

    }
}

#endif //HAZELCAST_TransactionalList

