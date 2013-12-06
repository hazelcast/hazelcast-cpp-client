//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionProxy
#define HAZELCAST_TransactionProxy

#include "hazelcast/client/spi/ClusterService.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        class TransactionOptions;

        namespace txn {

            class HAZELCAST_API TxnState {
            public:
                enum states {
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

                TxnState(states value);

                virtual operator int() const;

                virtual void operator = (int i);

                std::vector<states> values;
            };

            class HAZELCAST_API TransactionProxy {
            public:

                TransactionProxy(TransactionOptions &, spi::ClusterService &, serialization::SerializationService &, connection::Connection *);

                std::string getTxnId() const;

                TxnState getState() const;

                long getTimeoutMillis() const;

                void begin();

                void commit();

                void rollback();

                serialization::SerializationService &getSerializationService();

                spi::ClusterService &getClusterService();

                connection::Connection *getConnection();

                template <typename Response, typename Request>
                boost::shared_ptr<Response> sendAndReceive(const Request &request) {
                    return clusterService.sendAndReceiveFixedConnection<Response>(connection, request);
                };
            private:
                TransactionOptions &options;
                spi::ClusterService &clusterService;
                serialization::SerializationService &serializationService;
                connection::Connection *connection;
                long threadId;

                std::string txnId;
                TxnState state;
                long startTime;

                void closeConnection();

                void checkThread();

                void checkTimeout();


            };
        }
    }
}

#endif //HAZELCAST_TransactionProxy
