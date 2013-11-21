//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalQueue
#define HAZELCAST_TransactionalQueue

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/queue/TxnOfferRequest.h"
#include "hazelcast/client/exception/InterruptedException.h"
#include "hazelcast/client/queue/TxnPollRequest.h"
#include "hazelcast/client/queue/TxnSizeRequest.h"

namespace hazelcast {
    namespace client {
        template <typename E>
        class TransactionalQueue : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            bool offer(const E &e) {
                try {
                    return offer(e, 0);
                } catch (exception::InterruptedException &) {
                    return false;
                }
            };

            bool offer(const E &e, long timeoutInMillis) {
                serialization::Data data = toData(e);
                queue::TxnOfferRequest request(getName(), timeoutInMillis, data);
                bool result;
                try {
                    result = invoke<bool>(request);
                } catch(exception::ServerException &){
                    throw exception::InterruptedException("TransactionalQueue::offer", "timeout");
                }
                return result;
            };

            E poll() {
                try {
                    return poll(0);
                } catch (exception::InterruptedException &e) {
                    return E();
                }
            };

            E poll(long timeoutInMillis) {
                queue::TxnPollRequest request(getName(), timeoutInMillis);
                E result;
                try {
                    result = invoke<E>(request);
                } catch(exception::ServerException &e){
                    throw exception::InterruptedException("TransactionalQueue::poll", "timeout");
                }
                return result;
            };

            int size() {
                queue::TxnSizeRequest request(getName());
                return invoke<int>(request);
            }

            void onDestroy() {
            }

        private:
            TransactionalQueue(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }

            template<typename T>
            serialization::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename Response, typename Request>
            Response invoke(const Request &request) {
                return getContext().template sendAndReceive<Response>(request);
            };

        };

    }
}

#endif //HAZELCAST_TransactionalQueue
