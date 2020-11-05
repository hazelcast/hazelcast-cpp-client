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
#include <hazelcast/client/serialization/serialization.h>

#include "hazelcast/client/Address.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
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
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/MembershipListener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class InitialMembershipEvent;

        namespace connection {
             class ClientConnectionManagerImpl;
        };

        namespace security {
            class HAZELCAST_API credentials {
            public:
                enum credential_type {
                    username_password,
                    token
                };

                credentials(const std::string &name);

                virtual ~credentials();

                const std::string &name() const;

                virtual const credential_type type() const = 0;

            private:
                std::string name_;
            };

            class HAZELCAST_API username_password_credentials : public credentials {
            public:
                username_password_credentials(const std::string &name, const std::string &password);

                const std::string &password() const;

                const credential_type type() const override;

            private:
                std::string password_;
            };

            class HAZELCAST_API token_credentials : public credentials {
            public:
                token_credentials(const std::vector<byte> &token);

                const std::vector<byte> &token() const;

                const credential_type type() const override;

            private:
                std::vector<byte> token_;
            };
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
             * Returns the configured cluster name. The name is sent as part of client authentication message and may be verified on the
             * member.
             *
             * \return the configured cluster name
             */
            const std::string &get_cluster_name() const;

            ClientConfig &set_cluster_name(const std::string &clusterName);

            /**
            *  There are two types of credentials you can provide, \username_password_credentials and \token_credentials
            *
            *  \return itself ClientConfig
            */
            ClientConfig &set_credentials(const std::shared_ptr<security::credentials> &credential);

            const std::shared_ptr<security::credentials> &get_credentials() const;

            /**
            * If true, client will redo the operations that were executing on the server and client lost the connection.
            * This can be because of network, or simply because the member died. However it is not clear whether the
            * application is performed or not. For idempotent operations this is harmless, but for non idempotent ones
            * retrying can cause to undesirable effects. Note that the redo can perform on any member.
            *
            * If false, the operation will throw IOException.
            *
            * \param redoOperation
            * return itself ClientConfig
            */
            ClientConfig &set_redo_operation(bool redoOperation);

            /**
            *
            * see setRedoOperation
            * returns redoOperation
            */
            bool is_redo_operation() const;

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            *
            * \return itself ClientConfig
            */
            ClientConfig &set_socket_interceptor(SocketInterceptor &&interceptor);

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            */
            const SocketInterceptor &get_socket_interceptor() const;

            /**
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * \param listener LifecycleListener
            * \return itself ClientConfig
            */
            ClientConfig &add_listener(LifecycleListener &&listener);

            /**
            *
            * \return registered lifecycleListeners
            */
            const std::vector<LifecycleListener> &get_lifecycle_listeners() const;

            /**
            * Adds a listener to configuration to be registered when HazelcastClient starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * \param listener MembershipListener
            * \return itself ClientConfig
            */
            ClientConfig &add_listener(MembershipListener &&listener);

            /**
            * Returns registered membershipListeners
            *
            * \return registered membershipListeners
            */
            const std::vector<MembershipListener> &get_membership_listeners() const;

            /**
            * Used to distribute the operations to multiple Endpoints.
            *
            * \return loadBalancer
            */
            LoadBalancer *const get_load_balancer();

            /**
            * Used to distribute the operations to multiple Endpoints.
            * If not set, RoundRobin based load balancer is used
            *
            * \param loadBalancer
            *
            * \return itself ClientConfig
            */
            ClientConfig &set_load_balancer(LoadBalancer *loadBalancer);

            /**
            *
            *  \return serializationConfig
            */
            SerializationConfig &get_serialization_config();

            /**
            * SerializationConfig is used to
            *   * set version of portable classes in this client (@see versioned_portable_serializer)
            *
            * \param serializationConfig
            * \return itself ClientConfig
            */
            ClientConfig &set_serialization_config(SerializationConfig const &serializationConfig);

            /**
            * Gets a reference to properties map
            *
            * \return properties map
            */
            const std::unordered_map<std::string, std::string> &get_properties() const;

            /**
            * Sets the value of a named property
            *
            * @see ClientProperties for properties that is used to configure client
            *
            * \param name  property name
            * \param value value of the property
            * \return itself ClientConfig
            */
            ClientConfig &set_property(const std::string &name, const std::string &value);

            /**
             * Adds a ClientReliableTopicConfig.
             *
             * \param reliableTopicConfig the ReliableTopicConfig to add
             * \return configured {\link ClientConfig} for chaining
             */
            ClientConfig &add_reliable_topic_config(const config::ReliableTopicConfig &reliableTopicConfig);

            /**
             * Gets the ClientReliableTopicConfig for a given reliable topic name.
             *
             * \param name the name of the reliable topic
             * \return the found config. If none is found, a default configured one is returned.
             */
            const config::ReliableTopicConfig &get_reliable_topic_config(const std::string &name);

            /**
             * Helper method to add a new NearCacheConfig
             *
             * \param nearCacheConfig NearCacheConfig to be added
             * \return configured ClientConfig for chaining
             * @see NearCacheConfig
             * 
             * Memory ownership of the config is passed to the client config
             */
            ClientConfig &add_near_cache_config(const config::NearCacheConfig &nearCacheConfig);

            /**
             * Gets the NearCacheConfig configured for the map / cache with name
             *
             * \param name name of the map / cache
             * \return Configured NearCacheConfig
             * @see NearCacheConfig
             */
            const config::NearCacheConfig *get_near_cache_config(const std::string &name) const;

            /**
             * Gets {\link com.hazelcast.client.config.ClientNetworkConfig}
             *
             * \return {\link com.hazelcast.client.config.ClientNetworkConfig}
             * @see com.hazelcast.client.config.ClientNetworkConfig
             */
            config::ClientNetworkConfig &get_network_config();

            /**
             * Sets {\link com.hazelcast.client.config.ClientNetworkConfig}
             *
             * \param networkConfig {\link com.hazelcast.client.config.ClientNetworkConfig} to be set
             * \return configured ClientConfig for chaining
             * @see com.hazelcast.client.config.ClientNetworkConfig
             */
            ClientConfig &set_network_config(const config::ClientNetworkConfig &networkConfig);

            const std::shared_ptr<std::string> &get_instance_name() const;

            void set_instance_name(const std::shared_ptr<std::string> &instanceName);

            /**
             * Pool size for internal ExecutorService which handles responses etc.
             *
             * \return int Executor pool size.
             */
            int32_t get_executor_pool_size() const;

            /**
             * Sets Client side Executor pool size.
             *
             * \param executorPoolSize pool size
             * \return configured ClientConfig for chaining
             */
            void set_executor_pool_size(int32_t executorPoolSize);

            config::ClientConnectionStrategyConfig &get_connection_strategy_config();

            ClientConfig &
            set_connection_strategy_config(const config::ClientConnectionStrategyConfig &connectionStrategyConfig);

            /**
             * Returns a {\link ClientFlakeIdGeneratorConfig} configuration for the given flake ID generator name.
             * <p>
             * The name is matched by pattern to the configuration and by stripping the
             * partition ID qualifier from the given {@code name}.
             * If there is no config found by the name, it will return the configuration
             * with the name {@code "default"}.
             *
             * \param name name of the flake ID generator config
             * \return the flake ID generator configuration
             * @throws ConfigurationException if ambiguous configurations are found
             * @see StringPartitioningStrategy#getBaseName(std::string)
             * @see #setConfigPatternMatcher(ConfigPatternMatcher)
             * @see #getConfigPatternMatcher()
             */
            const config::ClientFlakeIdGeneratorConfig *find_flake_id_generator_config(const std::string &name);

            /**
             * Returns the {\link ClientFlakeIdGeneratorConfig} for the given name, creating
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
             * explicitly adding it by invoking {\link #addFlakeIdGeneratorConfig(ClientFlakeIdGeneratorConfig)}.
             * <p>
             * Because it adds new configurations if they are not already present,
             * this method is intended to be used before this config is used to
             * create a hazelcast instance. Afterwards, newly added configurations
             * may be ignored.
             *
             * \param name name of the flake ID generator config
             * \return the cache configuration
             * @throws ConfigurationException if ambiguous configurations are found
             * @see StringPartitioningStrategy#getBaseName(std::string)
             */
            const config::ClientFlakeIdGeneratorConfig *get_flake_id_generator_config(const std::string &name);

            /**
             * Adds a flake ID generator configuration. The configuration is saved under the config
             * name, which may be a pattern with which the configuration will be
             * obtained in the future.
             *
             * \param config the flake ID configuration
             * \return this config instance
             */
            ClientConfig &add_flake_id_generator_config(const config::ClientFlakeIdGeneratorConfig &config);

            /**
             *
             * \return The logger configuration.
             */
            config::LoggerConfig &get_logger_config();

            const std::unordered_set<std::string> &get_labels() const;

            ClientConfig &set_labels(const std::unordered_set<std::string> &labels);

            ClientConfig &add_label(const std::string &label);

            /**
             * This feature reduces number of hops and increase performance for smart clients.
             * It is enabled by default for smart clients.
             * This config has no effect for unisocket clients.
             *
             * @param enabled enables client to get backup acknowledgements directly from the member
             *                that backups are applied
             * @return configured \ClientConfig for chaining
             */
            ClientConfig &backup_acks_enabled(bool enabled);

            /**
             * Note that backup acks to client can be enabled only for smart client.
             * This config has no effect for unisocket clients.
             *
             * @return true if backup acknowledgements comes to client
             */
            bool backup_acks_enabled();

        private:
            std::string cluster_name_;

            config::ClientNetworkConfig networkConfig_;

            SerializationConfig serializationConfig_;

            LoadBalancer *loadBalancer_;

            impl::RoundRobinLB defaultLoadBalancer_;

            std::vector<MembershipListener> membershipListeners_;

            std::vector<LifecycleListener> lifecycleListeners_;

            std::unordered_map<std::string, std::string> properties_;

            bool redoOperation_;

            SocketInterceptor socketInterceptor_;

            std::shared_ptr<security::credentials> credentials_;

            std::unordered_map<std::string, config::ReliableTopicConfig> reliableTopicConfigMap_;

            std::unordered_map<std::string, config::NearCacheConfig> nearCacheConfigMap_;

            std::shared_ptr<std::string> instanceName_;

            /**
             * pool-size for internal ExecutorService which handles responses etc.
             */
            int32_t executorPoolSize_;

            config::ClientConnectionStrategyConfig connectionStrategyConfig_;

            std::unordered_map<std::string, config::ClientFlakeIdGeneratorConfig> flakeIdGeneratorConfigMap_;

            config::matcher::MatchingPointConfigPatternMatcher configPatternMatcher_;

            config::LoggerConfig loggerConfig_;

            std::unordered_set<std::string> labels_;

            bool backup_acks_enabled_ = true;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


