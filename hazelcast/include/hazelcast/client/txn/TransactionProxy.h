/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 8/5/13.





#ifndef HAZELCAST_TransactionProxy
#define HAZELCAST_TransactionProxy

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/CallFuture.h"
#include <boost/shared_ptr.hpp>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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
            namespace pimpl {
                class SerializationService;
            }
        }

        namespace txn {
            class BaseTxnRequest;

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

                void operator=(int i);

                std::vector<State> values;
            };

            class HAZELCAST_API TransactionProxy {
            public:

                TransactionProxy(TransactionOptions&, spi::ClientContext& clientContext, boost::shared_ptr<connection::Connection> connection);

                std::string getTxnId() const;

                TxnState getState() const;

                int getTimeoutSeconds() const;

                void begin();

                void commit();

                void rollback();

                serialization::pimpl::SerializationService& getSerializationService();

                spi::InvocationService& getInvocationService();

                boost::shared_ptr<connection::Connection> getConnection();

            private:

                TransactionOptions& options;
                spi::ClientContext& clientContext;
                boost::shared_ptr<connection::Connection> connection;

                long threadId;
                std::string txnId;

                TxnState state;
                time_t startTime;

                void onTxnEnd();

                serialization::pimpl::Data invoke(BaseTxnRequest *request);

                void checkThread();

                void checkTimeout();


            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_TransactionProxy

