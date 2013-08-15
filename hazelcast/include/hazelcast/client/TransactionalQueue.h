//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalQueue
#define HAZELCAST_TransactionalQueue

#include "Data.h"
#include "TransactionProxy.h"
#include "TxnOfferRequest.h"
#include "InterruptedException.h"
#include "TxnPollRequest.h"
#include "TxnSizeRequest.h"

namespace hazelcast {
    namespace client {
        template <typename E>
        class TransactionalQueue {
            friend class TransactionContext;

        public:
            bool offer(const E& e) {
                try {
                    return offer(e, 0);
                } catch (exception::InterruptedException& ex) {
                    return false;
                }
            };

            bool offer(const E& e, long timeoutInMillis) {
                serialization::Data data = toData(e);
                queue::TxnOfferRequest request(name, timeoutInMillis, data);
                bool result;
                try {
                    result = invoke<bool>(request);
                } catch(exception::ServerException& e){
                    throw exception::InterruptedException("TransactionalQueue::offer", "timeout");
                }
                return result;
            };

            E poll() {
                try {
                    return poll(0);
                } catch (exception::InterruptedException& e) {
                    return E();
                }
            };

            E poll(long timeoutInMillis) {
                queue::TxnPollRequest request(name, timeoutInMillis);
                E result;
                try {
                    result = invoke<E>(request);
                } catch(exception::ServerException& e){
                    throw exception::InterruptedException("TransactionalQueue::poll", "timeout");
                }
                return result;
            };

            int size() {
                queue::TxnSizeRequest request(name);
                return invoke<int>(request);
            }

            void destroy() {
                //TODO
            }

        private:
            txn::TransactionProxy *transaction;
            std::string name;

            void init(const std::string& name, txn::TransactionProxy *transactionProxy) {
                this->transaction = transactionProxy;
                this->name = name;
            };

            template<typename T>
            serialization::Data toData(const T& object) {
                return transaction->getSerializationService().toData<T>(&object);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return transaction->sendAndReceive<Response>(request);
            };

        };

    }
}

#endif //HAZELCAST_TransactionalQueue
