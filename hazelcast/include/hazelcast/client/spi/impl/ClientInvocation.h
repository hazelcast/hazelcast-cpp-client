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

#include <ostream>
#include <memory>
#include <atomic>

#include <boost/thread/future.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include "hazelcast/util/Sync.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"

#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }

    namespace client {
        class Address;

        namespace connection {
            class Connection;
        }

        namespace spi {
            class LifecycleService;

            class ClientContext;

            namespace impl {
                class ClientClusterServiceImpl;
                class ClientInvocationServiceImpl;
                class ClientExecutionServiceImpl;

                namespace sequence {
                    class CallIdSequence;
                }

                /**
                 * Handles the routing of a request from a Hazelcast client.
                 * <p>
                 * 1) Where should request be sent?<br>
                 * 2) Should it be retried?<br>
                 * 3) How many times is it retried?
                 */
                class HAZELCAST_API ClientInvocation
                        : public std::enable_shared_from_this<ClientInvocation> {
                public:
                    virtual ~ClientInvocation();

                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                    std::shared_ptr<protocol::ClientMessage> &&clientMessage,
                                                                    const std::string &objectName, int partitionId);


                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                    std::shared_ptr<protocol::ClientMessage> &&clientMessage,
                                                                    const std::string &objectName,
                                                                    const std::shared_ptr<connection::Connection> &connection = nullptr);


                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                    std::shared_ptr<protocol::ClientMessage> &&clientMessage,
                                                                    const std::string &objectName,
                                                                    boost::uuids::uuid uuid);

                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                    protocol::ClientMessage &clientMessage,
                                                                    const std::string &objectName, int partitionId);


                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                    protocol::ClientMessage &clientMessage,
                                                                    const std::string &objectName,
                                                                    const std::shared_ptr<connection::Connection> &connection = nullptr);


                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                    protocol::ClientMessage &clientMessage,
                                                                    const std::string &objectName,
                                                                    boost::uuids::uuid uuid);

                    boost::future<protocol::ClientMessage> invoke();

                    boost::future<protocol::ClientMessage> invokeUrgent();

                    void run();

                    virtual const std::string getName() const;

                    void notify(const std::shared_ptr<protocol::ClientMessage> &clientMessage);

                    void notifyException(std::exception_ptr exception);

                    std::shared_ptr<connection::Connection> getSendConnection() const;

                    std::shared_ptr<connection::Connection> getSendConnectionOrWait() const;

                    void
                    setSendConnection(const std::shared_ptr<connection::Connection> &sendConnection);

                    std::shared_ptr<protocol::ClientMessage> getClientMessage() const;

                    const std::shared_ptr<EventHandler < protocol::ClientMessage> > &getEventHandler() const;

                    void setEventHandler(const std::shared_ptr<EventHandler < protocol::ClientMessage>> &eventHandler);

                    friend std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation);

                    boost::promise<protocol::ClientMessage> &getPromise();

                private:
                    static constexpr int MAX_FAST_INVOCATION_COUNT = 5;
                    static constexpr int UNASSIGNED_PARTITION = -1;

                    util::ILogger &logger_;
                    LifecycleService &lifecycle_service_;
                    ClientClusterServiceImpl &client_cluster_service_;
                    ClientInvocationServiceImpl &invocation_service_;
                    std::shared_ptr<ClientExecutionServiceImpl> execution_service_;
                    boost::atomic_shared_ptr<std::shared_ptr<protocol::ClientMessage>> client_message_;
                    std::shared_ptr<sequence::CallIdSequence> call_id_sequence_;
                    boost::uuids::uuid uuid_;
                    int partition_id_;
                    std::chrono::steady_clock::time_point start_time_;
                    std::chrono::steady_clock::duration retry_pause_;
                    std::string object_name_;
                    std::shared_ptr<connection::Connection> connection_;
                    boost::atomic_shared_ptr<std::shared_ptr<connection::Connection>> send_connection_;
                    std::shared_ptr<EventHandler < protocol::ClientMessage>> event_handler_;
                    std::atomic<int64_t> invoke_count_;
                    boost::promise<protocol::ClientMessage> invocation_promise_;
                    bool urgent_;
                    bool smart_routing_;

                    ClientInvocation(spi::ClientContext &clientContext,
                                     std::shared_ptr<protocol::ClientMessage> &&message,
                                     const std::string &name, int partition = UNASSIGNED_PARTITION,
                                     const std::shared_ptr<connection::Connection> &conn = nullptr,
                                     boost::uuids::uuid uuid = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                                                0x0, 0x0, 0x0, 0x0, 0x0});

                    void invokeOnSelection();

                    bool isBindToSingleConnection() const;

                    void retry();

                    bool should_retry(exception::IException &exception);

                    void execute();

                    ClientInvocation(const ClientInvocation &rhs) = delete;

                    void operator=(const ClientInvocation &rhs) = delete;

                    std::shared_ptr<protocol::ClientMessage> copyMessage();

                    void setException(const exception::IException &e, boost::exception_ptr exceptionPtr);

                    void log_exception(exception::IException &e);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


