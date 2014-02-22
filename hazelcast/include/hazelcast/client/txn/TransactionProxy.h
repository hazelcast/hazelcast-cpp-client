//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionProxy
#define HAZELCAST_TransactionProxy

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/txn/BaseTxnRequest.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        class TransactionOptions;

        namespace spi {
            class ClientContext;

            class ClusterService;
        }

        namespace serialization {
            class SerializationService;
        }

        namespace txn {

            class HAZELCAST_API TxnState {
            public:
                enum State {
                    NO_TXN,
                    ACTIVE,
                    PREPARING,
                    PREPARED,
                    COMMITTING,
                    COMMITTED,
                    COMMIT_FAILED,
                    ROLLING_BACK,
                    ROLLED_BACK
                } value;

                TxnState(State value);

                operator int() const;

                void operator = (int i);

                std::vector<State> values;
            };

            class HAZELCAST_API TransactionProxy {
            public:

                TransactionProxy(TransactionOptions &, spi::ClientContext &clientContext, boost::shared_ptr<connection::Connection> connection);

                std::string getTxnId() const;

                TxnState getState() const;

                long getTimeoutMillis() const;

                void begin();

                void commit();

                void rollback();

                serialization::SerializationService &getSerializationService();

                spi::InvocationService &getInvocationService();

                boost::shared_ptr<connection::Connection> getConnection();

            private:

                spi::ClientContext &clientContext;
                TransactionOptions &options;
                boost::shared_ptr<connection::Connection> connection;

                long threadId;
                std::string txnId;

                TxnState state;
                long startTime;

                void onTxnEnd();

                template<typename Response>
                boost::shared_ptr<Response> invoke(BaseTxnRequest *request) {
                    request->setTxnId(txnId);
                    request->setThreadId(threadId);
                    spi::InvocationService &invocationService = clientContext.getInvocationService();
                    serialization::SerializationService &ss = clientContext.getSerializationService();
                    boost::shared_future<serialization::Data> future = invocationService.invokeOnConnection(request, connection);
                    return ss.toObject<Response>(future.get());
                }

                void checkThread();

                void checkTimeout();


            };
        }
    }
}

#endif //HAZELCAST_TransactionProxy
