//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionProxy
#define HAZELCAST_TransactionProxy

#include "hazelcast/util/HazelcastDll.h"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        class TransactionOptions;

        namespace spi{
            class ClientContext;

            class ClusterService;
        }

        namespace serialization{
            class SerializationService;
        }

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

                TransactionProxy(TransactionOptions &, spi::ClientContext& clientContext, connection::Connection* connection);

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
//TODO					return clusterService.sendAndReceiveFixedConnection<Response>(connection, request);
                    return boost::shared_ptr<Response>();
                };
            private:
                spi::ClientContext& clientContext;
                TransactionOptions &options;

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
