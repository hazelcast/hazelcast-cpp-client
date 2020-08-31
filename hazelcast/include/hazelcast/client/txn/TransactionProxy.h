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

#include <memory>
#include <vector>
#include <atomic>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/serialization.h"

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
        }

        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }

        namespace protocol {
            class ClientMessage;
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

                void operator=(int i);

                std::vector<State> values;
            };

            class HAZELCAST_API TransactionProxy {
            public:
                TransactionProxy(TransactionOptions&, spi::ClientContext& clientContext, std::shared_ptr<connection::Connection> connection);

                TransactionProxy(const TransactionProxy &rhs);

                boost::uuids::uuid getTxnId() const;

                TxnState getState() const;

                std::chrono::steady_clock::duration getTimeout() const;

                boost::future<void> begin();

                boost::future<void> commit();

                boost::future<void> rollback();

                serialization::pimpl::SerializationService& getSerializationService();

                std::shared_ptr<connection::Connection> getConnection();

                spi::ClientContext &getClientContext() const;

            private:
                TransactionOptions& options;
                spi::ClientContext& clientContext;
                std::shared_ptr<connection::Connection> connection;

                std::atomic<bool> TRANSACTION_EXISTS{ false };

                int64_t threadId;
                boost::uuids::uuid txnId;

                TxnState state;
                std::chrono::steady_clock::time_point startTime;

                void checkThread();

                void checkTimeout();

                boost::future<protocol::ClientMessage> invoke(protocol::ClientMessage &request);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



