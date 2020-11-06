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
                TransactionProxy(TransactionOptions&, spi::ClientContext& client_context, std::shared_ptr<connection::Connection> connection);

                TransactionProxy(const TransactionProxy &rhs);

                boost::uuids::uuid get_txn_id() const;

                TxnState get_state() const;

                std::chrono::milliseconds get_timeout() const;

                boost::future<void> begin();

                boost::future<void> commit();

                boost::future<void> rollback();

                serialization::pimpl::SerializationService& get_serialization_service();

                std::shared_ptr<connection::Connection> get_connection();

                spi::ClientContext &get_client_context() const;

            private:
                TransactionOptions& options_;
                spi::ClientContext& client_context_;
                std::shared_ptr<connection::Connection> connection_;

                std::atomic<bool> transaction_exists_{ false };

                int64_t thread_id_;
                boost::uuids::uuid txn_id_;

                TxnState state_;
                std::chrono::steady_clock::time_point start_time_;

                void check_thread();

                void check_timeout();

                boost::future<protocol::ClientMessage> invoke(protocol::ClientMessage &request);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



