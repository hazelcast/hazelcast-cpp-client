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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATION_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATION_H_

#include <ostream>
#include <memory>
#include <atomic>

#include "hazelcast/util/Runnable.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"

#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"

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

        namespace protocol {
            class ClientMessage;
        }

        namespace spi {
            class LifecycleService;

            class ClientClusterService;

            class ClientInvocationService;

            class ClientContext;

            class ClientExecutionService;

            namespace impl {
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
                        : public util::Runnable,
                          public std::enable_shared_from_this<ClientInvocation> {
                public:
                    virtual ~ClientInvocation();

                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName, int partitionId);


                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName,
                                                                      const std::shared_ptr<connection::Connection> &connection);


                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName, const Address &address);

                    static std::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName);

                    std::shared_ptr<ClientInvocationFuture> invoke();

                    std::shared_ptr<ClientInvocationFuture> invokeUrgent();

                    void run();

                    virtual const std::string getName() const;

                    void notify(const std::shared_ptr<protocol::ClientMessage> &clientMessage);

                    void notifyException(const std::shared_ptr<exception::IException> &exception);

                    std::shared_ptr<connection::Connection> getSendConnection();

                    std::shared_ptr<connection::Connection> getSendConnectionOrWait();

                    void
                    setSendConnection(const std::shared_ptr<connection::Connection> &sendConnection);

                    const std::shared_ptr<protocol::ClientMessage> getClientMessage();

                    const std::shared_ptr<EventHandler<protocol::ClientMessage> > &getEventHandler() const;

                    void setEventHandler(const std::shared_ptr<EventHandler<protocol::ClientMessage> > &eventHandler);

                    friend std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation);

                    static bool isRetrySafeException(exception::IException &exception);

                    std::shared_ptr<util::Executor> getUserExecutor();

                private:
                    /**
                     * Create an invocation that will be executed on owner of {@code partitionId}.
                     */
                    ClientInvocation(spi::ClientContext &clientContext,
                                     std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName, int partitionId);

                    /**
                     * Create an invocation that will be executed on given {@code connection}.
                     */
                    ClientInvocation(spi::ClientContext &clientContext,
                                     std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName,
                                     const std::shared_ptr<connection::Connection> &connection);

                    /**
                     * Create an invocation that will be executed on random member.
                     */
                    ClientInvocation(spi::ClientContext &clientContext,
                                     std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName);

                    /**
                     * Create an invocation that will be executed on member with given {@code address}.
                     */
                    ClientInvocation(spi::ClientContext &clientContext,
                                     std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName, const Address &address);

                    static void invokeOnSelection(const std::shared_ptr<ClientInvocation> &invocation);

                    bool isBindToSingleConnection() const;

                    void retry();

                    static const int MAX_FAST_INVOCATION_COUNT = 5;
                    static const int UNASSIGNED_PARTITION = -1;

                    util::ILogger &logger;
                    LifecycleService &lifecycleService;
                    ClientClusterService &clientClusterService;
                    ClientInvocationService &invocationService;
                    std::shared_ptr<ClientExecutionService> executionService;
                    util::Sync<std::shared_ptr<protocol::ClientMessage> > clientMessage;
                    std::shared_ptr<sequence::CallIdSequence> callIdSequence;
                    std::shared_ptr<Address> address;
                    int partitionId;
                    int64_t startTimeMillis;
                    int64_t retryPauseMillis;
                    std::string objectName;
                    std::shared_ptr<connection::Connection> connection;
                    util::Sync<std::shared_ptr<connection::Connection> > sendConnection;
                    std::shared_ptr<EventHandler<protocol::ClientMessage> > eventHandler;
                    std::atomic<int64_t> invokeCount;
                    std::shared_ptr<ClientInvocationFuture> clientInvocationFuture;

                    bool isNotAllowedToRetryOnSelection(exception::IException &exception);

                    std::shared_ptr<exception::OperationTimeoutException> newOperationTimeoutException(exception::IException &exception);

                    void execute();

                    ClientInvocation(const ClientInvocation &rhs);

                    void operator=(const ClientInvocation &rhs);

                    std::shared_ptr<protocol::ClientMessage> copyMessage();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATION_H_
