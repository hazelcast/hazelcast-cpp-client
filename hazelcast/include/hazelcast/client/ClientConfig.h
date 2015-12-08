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
#ifndef HAZELCAST_CLIENT_CONFIG
#define HAZELCAST_CLIENT_CONFIG

#include "hazelcast/client/Address.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/Credentials.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/util/ILogger.h"
#include <vector>
#include <set>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class MembershipListener;

        class InitialMembershipListener;

        class LifecycleListener;

        /**
        * HazelcastClient configuration class.
        */
        class HAZELCAST_API ClientConfig {
        public:

            /**
            * Constructor with default values.
            * smart(true)
            * redoOperation(false)
            * connectionTimeout(60000)
            * connectionAttemptLimit(2)
            * attemptPeriod(3000)
            * defaultLoadBalancer(impl::RoundRobinLB)
            */
            ClientConfig();

            /**
            * Adds an address to list of the initial addresses.
            * Client will use this list to find a running Member, connect to it.
            *
            * @param address
            * @return itself ClientConfig
            */
            ClientConfig& addAddress(const Address& address);

            /**
            * Adds all address in given vector to list of the initial addresses.
            * Client will use this list to find a running Member, connect to it.
            *
            * @param addresses vector of addresses
            * @return itself ClientConfig
            */
            ClientConfig& addAddresses(const std::vector<Address>& addresses);

            /**
            * Returns set of the initial addresses.
            * Client will use this vector to find a running Member, connect to it.
            *
            * @return vector of addresses
            */
            std::set<Address, addressComparator>& getAddresses();

            /**
            * The Group Configuration properties like:
            * Name and Password that is used to connect to the cluster.
            *
            * @param groupConfig
            * @return itself ClientConfig
            */
            ClientConfig& setGroupConfig(GroupConfig& groupConfig);

            /**
            *
            * @return groupConfig
            */
            GroupConfig& getGroupConfig();

            /**
            * Can be used instead of GroupConfig in Hazelcast Extensions.
            *
            *  @return itself ClientConfig
            */
            ClientConfig& setCredentials(Credentials *credentials);

            /**
            * Can be used instead of GroupConfig in Hazelcast Extensions.
            */
            const Credentials *getCredentials();

            /**
            * While client is trying to connect initially to one of the members in the ClientConfig#addressList,
            * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
            * attempt to retry as much as ClientConfig#connectionAttemptLimit times.
            *
            * @param connectionAttemptLimit
            * @return itself ClientConfig
            */
            ClientConfig& setConnectionAttemptLimit(int connectionAttemptLimit);

            /**
            * While client is trying to connect initially to one of the members in the ClientConfig#addressList,
            * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
            * attempt to retry as much as ClientConfig#connectionAttemptLimit times.
            *
            * return int connectionAttemptLimit
            */
            int getConnectionAttemptLimit() const;

            /**
            * @param connectionTimeout Timeout value in millis for nodes to accept client connection requests.
            *                          A zero value means wait until connection established or an error occurs.
            *
            * @return itself ClientConfig
            */
            ClientConfig& setConnectionTimeout(int connectionTimeoutInMillis);

            /**
            * Timeout value for nodes to accept client connection requests.
            *
            * @return int connectionTimeoutInMillis
            */
            int getConnectionTimeout() const;

            /**
            * Period for the next attempt to find a member to connect. (see ClientConfig#connectionAttemptLimit ).
            *
            * @param attemptPeriodInMillis
            * @return itself ClientConfig
            */
            ClientConfig& setAttemptPeriod(int attemptPeriodInMillis);

            /**
            * Period for the next attempt to find a member to connect. (see ClientConfig#connectionAttemptLimit ).
            *
            * @return int attemptPeriodInMillis
            */
            int getAttemptPeriod() const;

            /**
            * If true, client will redo the operations that were executing on the server and client lost the connection.
            * This can be because of network, or simply because the member died. However it is not clear whether the
            * application is performed or not. For idempotent operations this is harmless, but for non idempotent ones
            * retrying can cause to undesirable effects. Note that the redo can perform on any member.
            *
            * If false, the operation will throw IOException.
            *
            * @param redoOperation
            * return itself ClientConfig
            */
            ClientConfig& setRedoOperation(bool redoOperation);

            /**
            *
            * see setRedoOperation
            * returns redoOperation
            */
            bool isRedoOperation() const;

            /**
            * @return true if client configured as smart
            * see setSmart()
            */
            bool isSmart() const;

            /**
            * If true, client will route the key based operations to owner of the key at the best effort.
            * Note that it uses a cached version of PartitionService#getPartitions() and doesn't
            * guarantee that the operation will always be executed on the owner.
            * The cached table is updated every 10 seconds.
            *
            * @param smart
            *
            * @return itself ClientConfig
            */
            ClientConfig& setSmart(bool smart);

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            *
            * @return itself ClientConfig
            */
            ClientConfig& setSocketInterceptor(SocketInterceptor *socketInterceptor);

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            */
            SocketInterceptor* getSocketInterceptor();

            /**
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener LifecycleListener *listener
            * @return itself ClientConfig
            */
            ClientConfig& addListener(LifecycleListener *listener);

            /**
            *
            * @return registered lifecycleListeners
            */
            const std::set<LifecycleListener *>& getLifecycleListeners() const;

            /**
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener MembershipListener *listener
            * @return itself ClientConfig
            */
            ClientConfig& addListener(MembershipListener *listener);

            /**
            * Returns registered membershipListeners
            *
            * @return registered membershipListeners
            */
            const std::set<MembershipListener *>& getMembershipListeners() const;

            /**
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            *
            * @param listener InitialMembershipListener *listener
            * @return itself ClientConfig
            */
            ClientConfig& addListener(InitialMembershipListener *listener);

            /**
            * Returns registered initialMembershipListeners
            *
            * @return registered initialMembershipListeners
            */
            const std::set<InitialMembershipListener *>& getInitialMembershipListeners() const;

            /**
            * Used to distribute the operations to multiple Endpoints.
            *
            * @return loadBalancer
            */
            LoadBalancer *const getLoadBalancer();

            /**
            * Used to distribute the operations to multiple Endpoints.
            * If not set, RoundRobin based load balancer is used
            *
            * @param loadBalancer
            *
            * @return itself ClientConfig
            */
            ClientConfig& setLoadBalancer(LoadBalancer *loadBalancer);

            /**
            *  enum LogLevel { SEVERE = 100, WARNING = 90, INFO = 50 };
            *  set INFO to see every log.
            *  set WARNING to see only possible warnings and serious errors.
            *  set SEVERE to see only serious errors
            *
            *  Default log level is INFO
            * @return itself ClientConfig
            */
            ClientConfig& setLogLevel(LogLevel loggerLevel);


            /**
            *
            *  @return serializationConfig
            */
            SerializationConfig const& getSerializationConfig() const;

            /**
            * SerializationConfig is used to
            *   * set version of portable classes in this client (@see Portable)
            *   * register custom serializers to be used (@see Serializer , @see SerializationConfig#registerSerializer)
            *
            *
            * @param serializationConfig
            * @return itself ClientConfig
            */
            ClientConfig& setSerializationConfig(SerializationConfig const& serializationConfig);

            /**
            * Gets a reference to properties map
            *
            * @return properties map
            */
            std::map<std::string, std::string>& getProperties();

            /**
            * Sets the value of a named property
            *
            * @see ClientProperties for properties that is used to configure client
            *
            * @param name  property name
            * @param value value of the property
            * @return itself ClientConfig
            */
            ClientConfig& setProperty(const std::string& name, const std::string& value);

        private:

            GroupConfig groupConfig;

            SerializationConfig serializationConfig;

            std::set<Address, addressComparator> addressList;

            LoadBalancer *loadBalancer;

            std::auto_ptr<impl::RoundRobinLB> defaultLoadBalancer;

            std::set<MembershipListener *> membershipListeners;

            std::set<InitialMembershipListener *> initialMembershipListeners;

            std::set<LifecycleListener *> lifecycleListeners;

            std::map<std::string, std::string> properties;

            bool smart;

            bool redoOperation;

            int connectionTimeout;

            int connectionAttemptLimit;

            int attemptPeriod;

            SocketInterceptor *socketInterceptor;

            Credentials *credentials;

            std::auto_ptr<Credentials> defaultCredentials;

        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG */
