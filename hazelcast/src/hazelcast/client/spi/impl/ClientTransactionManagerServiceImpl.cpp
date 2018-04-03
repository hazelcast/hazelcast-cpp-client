/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include <ostream>

#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                ClientContext &impl::ClientTransactionManagerServiceImpl::getClient() const {
                    return client;
                }

                ClientTransactionManagerServiceImpl::ClientTransactionManagerServiceImpl(ClientContext &client,
                                                                                         hazelcast::client::LoadBalancer &loadBalancer)
                        : client(client), loadBalancer(loadBalancer) {}

                boost::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::connect() {
                    AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) client.getInvocationService();
                    int64_t startTimeMillis = util::currentTimeMillis();
                    int64_t invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                    ClientConfig &clientConfig = client.getClientConfig();
                    bool smartRouting = clientConfig.getNetworkConfig().isSmartRouting();

                    while (client.getLifecycleService().isRunning()) {
                        try {
                            if (smartRouting) {
                                return tryConnectSmart();
                            } else {
                                return tryConnectUnisocket();
                            }
                        } catch (exception::HazelcastClientOfflineException &) {
                            throw;
                        } catch (exception::IException &e) {
                            if (util::currentTimeMillis() - startTimeMillis > invocationTimeoutMillis) {
                                throw newOperationTimeoutException(e, invocationTimeoutMillis, startTimeMillis);
                            }
                        }
                        util::sleepmillis(invocationService.getInvocationRetryPauseMillis());
                    }
                    throw exception::HazelcastClientNotActiveException("ClientTransactionManagerServiceImpl::connect",
                                                                       "Client is shutdown");
                }

                boost::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::tryConnectSmart() {
                    try {
                        Address address = getRandomAddress();
                        return client.getConnectionManager().getOrConnect(address);
                    } catch (exception::IException &e) {
                        throwException(true);
                    }
                    return boost::shared_ptr<connection::Connection>();
                }

                Address ClientTransactionManagerServiceImpl::getRandomAddress() {
                    Member member = loadBalancer.next();
                    return member.getAddress();
                }

                boost::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::tryConnectUnisocket() {
                    boost::shared_ptr<connection::Connection> connection = client.getConnectionManager().getOwnerConnection();

                    if (connection.get()) {
                        return connection;
                    }
                    return throwException(false);
                }

                boost::shared_ptr<connection::Connection>
                ClientTransactionManagerServiceImpl::throwException(bool smartRouting) {
                    ClientConfig clientConfig = client.getClientConfig();
                    const config::ClientConnectionStrategyConfig &connectionStrategyConfig = clientConfig.getConnectionStrategyConfig();
                    config::ClientConnectionStrategyConfig::ReconnectMode reconnectMode = connectionStrategyConfig.getReconnectMode();
                    if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC) {
                        throw exception::HazelcastClientOfflineException(
                                "ClientTransactionManagerServiceImpl::throwException", "Hazelcast client is offline");
                    }
                    if (smartRouting) {
                        std::vector<Member> members = client.getCluster().getMembers();
                        std::ostringstream msg;
                        if (members.empty()) {
                            msg
                                    << "No address was return by the LoadBalancer since there are no members in the cluster";
                        } else {
                            msg << "No address was return by the LoadBalancer. "
                                << "But the cluster contains the following members:{";
                            for (std::vector<Member>::const_iterator it = members.begin();;) {
                                msg << (*it);

                                ++it;

                                if (it != members.end()) {
                                    msg << " , ";
                                } else {
                                    break;
                                }
                            }
                            msg << "}";
                        }
                        throw exception::IllegalStateException("ClientTransactionManagerServiceImpl::throwException",
                                                               msg.str());
                    }
                    throw exception::IllegalStateException("ClientTransactionManagerServiceImpl::throwException",
                                                           "No active connection is found");
                }

                exception::OperationTimeoutException
                ClientTransactionManagerServiceImpl::newOperationTimeoutException(exception::IException &throwable,
                                                                                  int64_t invocationTimeoutMillis,
                                                                                  int64_t startTimeMillis) {
                    std::ostringstream sb;
                    sb
                            << "Creating transaction context timed out because exception occurred after client invocation timeout "
                            << invocationTimeoutMillis << " ms. " << "Current time: "
                            << util::StringUtil::timeToString(util::currentTimeMillis()) << ". " << "Start time: "
                            << util::StringUtil::timeToString(startTimeMillis) << ". Total elapsed time: "
                            << (util::currentTimeMillis() - startTimeMillis) << " ms. ";
                    return exception::OperationTimeoutException(
                            "ClientTransactionManagerServiceImpl::newOperationTimeoutException", sb.str(),
                            boost::shared_ptr<exception::IException>(throwable.clone()));

                }
            }
        }
    }
}
