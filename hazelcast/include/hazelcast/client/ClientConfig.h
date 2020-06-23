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

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <boost/optional.hpp>

#include "hazelcast/client/Address.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/config/ReliableTopicConfig.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"
#include "hazelcast/client/config/matcher/MatchingPointConfigPatternMatcher.h"
#include "hazelcast/client/internal/config/ConfigUtils.h"
#include "hazelcast/client/config/LoggerConfig.h"
#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class MembershipListener;

        class InitialMembershipListener;

        class LifecycleListener;

        class InitialMembershipEvent;

        namespace connection {
             class ClientConnectionManagerImpl;
        };

        /**
        * HazelcastClient configuration class.
        */
        class HAZELCAST_API ClientConfig {
            friend class spi::impl::ClientClusterServiceImpl;
            friend class connection::ClientConnectionManagerImpl;
        public:

            /**
            * Constructor with default values.
            * smart(true)
            * redoOperation(false)
            * connectionAttemptLimit(2)
            * attemptPeriod(3000)
            * defaultLoadBalancer(impl::RoundRobinLB)
            */
            ClientConfig();

            /**
            * @deprecated Please use {@link ClientNetworkConfig#addAddress(const Address &)}.
            *
            * Adds an address to list of the initial addresses.
            * Client will use this list to find a running Member, connect to it.
            *
            * @param address
            * @return itself ClientConfig
            */
            ClientConfig &addAddress(const Address &address);

            /**
            * @deprecated Please use {@link ClientNetworkConfig#addAddresses(const std::vector<Address> &)}.
            *
            * Adds all address in given vector to list of the initial addresses.
            * Client will use this list to find a running Member, connect to it.
            *
            * @param addresses vector of addresses
            * @return itself ClientConfig
            */
            ClientConfig &addAddresses(const std::vector<Address> &addresses);

            /**
            * @deprecated Please use {@link ClientNetworkConfig#getAddresses()}.
            *
            * Returns set of the initial addresses.
            * Client will use this vector to find a running Member, connect to it.
            *
            * @return vector of addresses
            */
            std::unordered_set<Address> getAddresses();

            /**
            * The Group Configuration properties like:
            * Name and Password that is used to connect to the cluster.
            *
            * @param groupConfig
            * @return itself ClientConfig
            */
            ClientConfig &setGroupConfig(const GroupConfig &groupConfig);

            /**
            *
            * @return groupConfig
            */
            GroupConfig &getGroupConfig();

            /**
            * Can be used instead of GroupConfig in Hazelcast Extensions.
            *
            *  @return itself ClientConfig
            */
            template<typename T>
            ClientConfig &setCredentials(const T &credential) {
                serialization::pimpl::SerializationService ss(serializationConfig);
                credentials = ss.toData<T>(credential);
                principal = credential.getPrincipal();
                return *this;
            }

            /**
            * Gets the principal if set by the credentials
            */
            const boost::optional<std::string> &getPrincipal() const;

            /**
            * @deprecated Please use {@link ClientNetworkConfig#setConnectionAttemptLimit(int32_t)}
            *
            * While client is trying to connect initially to one of the members in the ClientConfig#addressList,
            * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
            * attempt to retry as much as ClientConfig#connectionAttemptLimit times.
            *
            * @param connectionAttemptLimit
            * @return itself ClientConfig
            */
            ClientConfig &setConnectionAttemptLimit(int connectionAttemptLimit);

            /**
            * @deprecated Please use {@link ClientNetworkConfig#getConnectionAttemptLimit()}
            *
            * While client is trying to connect initially to one of the members in the ClientConfig#addressList,
            * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
            * attempt to retry as much as ClientConfig#connectionAttemptLimit times.
            *
            * return int connectionAttemptLimit
            */
            int getConnectionAttemptLimit() const;

            /**
             * Use {@link ClientNetworkConfig#setConnectionTimeout} instead
             * @Deprecated
             */
            ClientConfig &setConnectionTimeout(int connectionTimeoutInMillis);

            /**
             * Use {@link ClientNetworkConfig#getConnectionTimeout} instead
             * @Deprecated
             */
            int getConnectionTimeout() const;

            /**
            * @deprecated Please use {@link ClientNetworkConfig#setAttemptPeriod(int32_t)}
            *
            * Period for the next attempt to find a member to connect. (see ClientConfig#connectionAttemptLimit ).
            *
            * @param attemptPeriodInMillis
            * @return itself ClientConfig
            */
            ClientConfig &setAttemptPeriod(int attemptPeriodInMillis);

            /**
            * @deprecated Please use {@link ClientNetworkConfig#getAttemptPeriod()}
            *
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
            ClientConfig &setRedoOperation(bool redoOperation);

            /**
            *
            * see setRedoOperation
            * returns redoOperation
            */
            bool isRedoOperation() const;

            /**
            * @deprecated Please use ClientNetworkConfig#isSmartRouting
            *
            * @return true if client configured as smart
            * see setSmart()
            */
            bool isSmart() const;

            /**
            * @deprecated Please use ClientNetworkConfig#setSmartRouting
            *
            * If true, client will route the key based operations to owner of the key at the best effort.
            * Note that it uses a cached version of PartitionService#getPartitions() and doesn't
            * guarantee that the operation will always be executed on the owner.
            * The cached table is updated every 10 seconds.
            *
            * @param smart
            *
            * @return itself ClientConfig
            */
            ClientConfig &setSmart(bool smart);

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            *
            * @return itself ClientConfig
            */
            ClientConfig &setSocketInterceptor(SocketInterceptor *interceptor);

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            */
            SocketInterceptor *getSocketInterceptor();

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
            ClientConfig &addListener(LifecycleListener *listener);

            /**
            *
            * @return registered lifecycleListeners
            */
            const std::unordered_set<LifecycleListener *> &getLifecycleListeners() const;

            /**
            * @deprecated Please use addListener(const std::shared_ptr<MembershipListener> &listener) instead.
            *
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener MembershipListener *listener
            * @return itself ClientConfig
            */
            ClientConfig &addListener(MembershipListener *listener);

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
            ClientConfig &addListener(const std::shared_ptr<MembershipListener> &listener);

            /**
            * Returns registered membershipListeners
            *
            * @return registered membershipListeners
            */
            const std::unordered_set<MembershipListener *> &getMembershipListeners() const;

            /**
             * Returns registered membershipListeners
             *
             * @return registered membership listeners. This method returns the same list as the
             * getMembershipListeners method but as a set of shared_ptr.
             */
            const std::unordered_set<std::shared_ptr<MembershipListener> > &getManagedMembershipListeners() const;

            /**
            * @deprecated Please use addListener(const std::shared_ptr<InitialMembershipListener> &listener)
            *
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            *
            * @param listener InitialMembershipListener *listener
            * @return itself ClientConfig
            */
            ClientConfig &addListener(InitialMembershipListener *listener);

            /**
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            *
            * @param listener InitialMembershipListener *listener
            * @return itself ClientConfig
            */
            ClientConfig &addListener(const std::shared_ptr<InitialMembershipListener> &listener);

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
            ClientConfig &setLoadBalancer(LoadBalancer *loadBalancer);

            /**
            *  enum LogLevel { SEVERE, WARNING, INFO, FINEST };
            *  set INFO to see every log.
            *  set WARNING to see only possible warnings and serious errors.
            *  set SEVERE to see only serious errors
            *  set FINEST to see all messages including debug messages.
            *
            * @param loggerLevel The log level to be set.
            * @return The configured client configuration for chaining.
             */
            ClientConfig &setLogLevel(LogLevel loggerLevel);

            /**
            *
            *  @return serializationConfig
            */
            SerializationConfig &getSerializationConfig();

            /**
            * SerializationConfig is used to
            *   * set version of portable classes in this client (@see versioned_portable_serializer)
            *
            * @param serializationConfig
            * @return itself ClientConfig
            */
            ClientConfig &setSerializationConfig(SerializationConfig const &serializationConfig);

            /**
            * Gets a reference to properties map
            *
            * @return properties map
            */
            std::unordered_map<std::string, std::string> &getProperties();

            /**
            * Sets the value of a named property
            *
            * @see ClientProperties for properties that is used to configure client
            *
            * @param name  property name
            * @param value value of the property
            * @return itself ClientConfig
            */
            ClientConfig &setProperty(const std::string &name, const std::string &value);

            /**
             * Adds a ClientReliableTopicConfig.
             *
             * @param reliableTopicConfig the ReliableTopicConfig to add
             * @return configured {@link ClientConfig} for chaining
             */
            ClientConfig &addReliableTopicConfig(const config::ReliableTopicConfig &reliableTopicConfig);

            /**
             * Gets the ClientReliableTopicConfig for a given reliable topic name.
             *
             * @param name the name of the reliable topic
             * @return the found config. If none is found, a default configured one is returned.
             */
            const config::ReliableTopicConfig *getReliableTopicConfig(const std::string &name);

            /**
             * Helper method to add a new NearCacheConfig
             *
             * @param nearCacheConfig {@link com.hazelcast.config.NearCacheConfig} to be added
             * @return configured {@link com.hazelcast.client.config.ClientConfig} for chaining
             * @see com.hazelcast.config.NearCacheConfig
             * 
             * Memory ownership of the config is passed to the client config
             */
            template <typename K, typename V>
            ClientConfig &addNearCacheConfig(const std::shared_ptr<config::NearCacheConfig<K, V> > nearCacheConfig) {
                nearCacheConfigMap.put(nearCacheConfig->getName(), nearCacheConfig);
                return *this;
            }

            /**
             * Gets the {@link NearCacheConfig} configured for the map / cache with name
             *
             * @param name name of the map / cache
             * @return Configured {@link NearCacheConfig}
             * @see com.hazelcast.config.NearCacheConfig
             */
            template <typename K, typename V>
            const std::shared_ptr<config::NearCacheConfig<K, V> > getNearCacheConfig(const std::string &name) {
                std::shared_ptr<config::NearCacheConfigBase> nearCacheConfig = internal::config::ConfigUtils::lookupByPattern<config::NearCacheConfigBase>(
                        configPatternMatcher, nearCacheConfigMap, name);
                if (nearCacheConfig.get() == NULL) {
                    nearCacheConfig = nearCacheConfigMap.get("default");
                }
                // not needed for c++ client since it is always native memory
                //initDefaultMaxSizeForOnHeapMaps(nearCacheConfig);
                return std::static_pointer_cast<config::NearCacheConfig<K, V> >(nearCacheConfig);
            }

            /**
             * Gets {@link com.hazelcast.client.config.ClientNetworkConfig}
             *
             * @return {@link com.hazelcast.client.config.ClientNetworkConfig}
             * @see com.hazelcast.client.config.ClientNetworkConfig
             */
            config::ClientNetworkConfig &getNetworkConfig();

            /**
             * Sets {@link com.hazelcast.client.config.ClientNetworkConfig}
             *
             * @param networkConfig {@link com.hazelcast.client.config.ClientNetworkConfig} to be set
             * @return configured {@link com.hazelcast.client.config.ClientConfig} for chaining
             * @see com.hazelcast.client.config.ClientNetworkConfig
             */
            ClientConfig &setNetworkConfig(const config::ClientNetworkConfig &networkConfig);

            const std::shared_ptr<std::string> &getInstanceName() const;

            void setInstanceName(const std::shared_ptr<std::string> &instanceName);

            /**
             * Pool size for internal ExecutorService which handles responses etc.
             *
             * @return int Executor pool size.
             */
            int32_t getExecutorPoolSize() const;

            /**
             * Sets Client side Executor pool size.
             *
             * @param executorPoolSize pool size
             * @return configured {@link com.hazelcast.client.config.ClientConfig} for chaining
             */
            void setExecutorPoolSize(int32_t executorPoolSize);

            config::ClientConnectionStrategyConfig &getConnectionStrategyConfig();

            ClientConfig &
            setConnectionStrategyConfig(const config::ClientConnectionStrategyConfig &connectionStrategyConfig);

            /**
             * Returns a {@link ClientFlakeIdGeneratorConfig} configuration for the given flake ID generator name.
             * <p>
             * The name is matched by pattern to the configuration and by stripping the
             * partition ID qualifier from the given {@code name}.
             * If there is no config found by the name, it will return the configuration
             * with the name {@code "default"}.
             *
             * @param name name of the flake ID generator config
             * @return the flake ID generator configuration
             * @throws ConfigurationException if ambiguous configurations are found
             * @see StringPartitioningStrategy#getBaseName(std::string)
             * @see #setConfigPatternMatcher(ConfigPatternMatcher)
             * @see #getConfigPatternMatcher()
             */
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> findFlakeIdGeneratorConfig(const std::string &name);

            /**
             * Returns the {@link ClientFlakeIdGeneratorConfig} for the given name, creating
             * one if necessary and adding it to the collection of known configurations.
             * <p>
             * The configuration is found by matching the the configuration name
             * pattern to the provided {@code name} without the partition qualifier
             * (the part of the name after {@code '@'}).
             * If no configuration matches, it will create one by cloning the
             * {@code "default"} configuration and add it to the configuration
             * collection.
             * <p>
             * This method is intended to easily and fluently create and add
             * configurations more specific than the default configuration without
             * explicitly adding it by invoking {@link #addFlakeIdGeneratorConfig(ClientFlakeIdGeneratorConfig)}.
             * <p>
             * Because it adds new configurations if they are not already present,
             * this method is intended to be used before this config is used to
             * create a hazelcast instance. Afterwards, newly added configurations
             * may be ignored.
             *
             * @param name name of the flake ID generator config
             * @return the cache configuration
             * @throws ConfigurationException if ambiguous configurations are found
             * @see StringPartitioningStrategy#getBaseName(std::string)
             */
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> getFlakeIdGeneratorConfig(const std::string &name);

            /**
             * Adds a flake ID generator configuration. The configuration is saved under the config
             * name, which may be a pattern with which the configuration will be
             * obtained in the future.
             *
             * @param config the flake ID configuration
             * @return this config instance
             */
            ClientConfig &addFlakeIdGeneratorConfig(const std::shared_ptr<config::ClientFlakeIdGeneratorConfig> &config);


            /**
             *
             * @return The logger configuration.
             */
            config::LoggerConfig &getLoggerConfig();

        private:
            GroupConfig groupConfig;

            config::ClientNetworkConfig networkConfig;

            SerializationConfig serializationConfig;

            LoadBalancer *loadBalancer;

            impl::RoundRobinLB defaultLoadBalancer;

            std::unordered_set<MembershipListener *> membershipListeners;
            std::unordered_set<std::shared_ptr<MembershipListener> > managedMembershipListeners;

            std::unordered_set<LifecycleListener *> lifecycleListeners;

            std::unordered_map<std::string, std::string> properties;

            bool redoOperation;

            SocketInterceptor *socketInterceptor;

            boost::optional<serialization::pimpl::Data> credentials;
            boost::optional<std::string> principal;

            std::unordered_map<std::string, config::ReliableTopicConfig> reliableTopicConfigMap;

            util::SynchronizedMap<std::string, config::NearCacheConfigBase> nearCacheConfigMap;

            std::shared_ptr<std::string> instanceName;

            /**
             * pool-size for internal ExecutorService which handles responses etc.
             */
            int32_t executorPoolSize;

            config::ClientConnectionStrategyConfig connectionStrategyConfig;

            util::SynchronizedMap<std::string, config::ClientFlakeIdGeneratorConfig> flakeIdGeneratorConfigMap;

            config::matcher::MatchingPointConfigPatternMatcher configPatternMatcher;

            config::LoggerConfig loggerConfig;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


