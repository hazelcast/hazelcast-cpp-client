/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/address.h"
#include "hazelcast/client/serialization_config.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/client/load_balancer.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/config/reliable_topic_config.h"
#include "hazelcast/client/config/near_cache_config.h"
#include "hazelcast/client/config/client_network_config.h"
#include "hazelcast/client/config/client_connection_strategy_config.h"
#include "hazelcast/client/config/client_flake_id_generator_config.h"
#include "hazelcast/client/config/matcher/matching_point_config_pattern_matcher.h"
#include "hazelcast/client/internal/config/ConfigUtils.h"
#include "hazelcast/client/config/logger_config.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/membership_listener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class initial_membership_event;

        namespace connection {
             class ClientConnectionManagerImpl;
        };

        namespace proxy { class ReliableTopicImpl; };

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
        * hazelcast_client configuration class.
        */
        class HAZELCAST_API client_config {
            friend class spi::impl::ClientClusterServiceImpl;
            friend class connection::ClientConnectionManagerImpl;
        public:

            /**
            * Constructor with default values.
            * smart(true)
            * redoOperation(false)
            * connectionAttemptLimit(2)
            * attemptPeriod(3000)
            * defaultLoadBalancer: round robin load balancer
            */
            client_config();

            client_config(const client_config &rhs) = delete;

            client_config &operator=(const client_config &rhs) = delete;

            client_config(client_config &&rhs);

            client_config &operator=(client_config &&rhs);

            /**
             * Returns the configured cluster name. The name is sent as part of client authentication message and may be verified on the
             * member.
             *
             * \return the configured cluster name
             */
            const std::string &get_cluster_name() const;

            client_config &set_cluster_name(const std::string &cluster_name);

            /**
            *  There are two types of credentials you can provide, \username_password_credentials and \token_credentials
            *
            *  \return itself ClientConfig
            */
            client_config &set_credentials(const std::shared_ptr<security::credentials> &credential);

            const std::shared_ptr<security::credentials> &get_credentials() const;

            /**
            * If true, client will redo the operations that were executing on the server and client lost the connection.
            * This can be because of network, or simply because the member died. However it is not clear whether the
            * application is performed or not. For idempotent operations this is harmless, but for non idempotent ones
            * retrying can cause to undesirable effects. Note that the redo can perform on any member.
            *
            * If false, the operation will throw io_exception.
            *
            * \param redoOperation
            * return itself ClientConfig
            */
            client_config &set_redo_operation(bool redo_operation);

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
            client_config &set_socket_interceptor(socket_interceptor &&interceptor);

            /**
            * Will be called with the Socket, each time client creates a connection to any Member.
            */
            const socket_interceptor &get_socket_interceptor() const;

            /**
            * Adds a listener to configuration to be registered when hazelcast_client starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * \param listener lifecycle_listener
            * \return itself ClientConfig
            */
            client_config &add_listener(lifecycle_listener &&listener);

            /**
            *
            * \return registered lifecycleListeners
            */
            const std::vector<lifecycle_listener> &get_lifecycle_listeners() const;

            /**
            * Adds a listener to configuration to be registered when hazelcast_client starts.
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * \param listener MembershipListener
            * \return itself ClientConfig
            */
            client_config &add_listener(membership_listener &&listener);

            /**
            * Returns registered membershipListeners
            *
            * \return registered membershipListeners
            */
            const std::vector<membership_listener> &get_membership_listeners() const;

            /**
            * Used to distribute the operations to multiple Endpoints.
            *
            * \return load_balancer
            */
            load_balancer &get_load_balancer();

            /**
            * Used to distribute the operations to multiple connections.
            * If not set, round robin based load balancer is used
            *
            * \param load_balancer
            *
            * \return itself ClientConfig
            */
            client_config &set_load_balancer(load_balancer &&load_balancer);

            /**
            *
            *  \return serializationConfig
            */
            serialization_config &get_serialization_config();

            /**
            * SerializationConfig is used to
            *   * set version of portable classes in this client (@see versioned_portable_serializer)
            *
            * \param serializationConfig
            * \return itself ClientConfig
            */
            client_config &set_serialization_config(serialization_config const &serialization_config);

            /**
            * Gets a reference to properties map
            *
            * \return properties map
            */
            const std::unordered_map<std::string, std::string> &get_properties() const;

            /**
            * Sets the value of a named property
            *
            * @see client_properties for properties that is used to configure client
            *
            * \param name  property name
            * \param value value of the property
            * \return itself ClientConfig
            */
            client_config &set_property(const std::string &name, const std::string &value);

            /**
             * Adds a ClientReliableTopicConfig.
             *
             * \param reliableTopicConfig the ReliableTopicConfig to add
             * \return configured {\link ClientConfig} for chaining
             */
            client_config &add_reliable_topic_config(const config::reliable_topic_config &reliable_topic_config);

            /**
             * Gets the ClientReliableTopicConfig for a given reliable topic name.
             *
             * \param name the name of the reliable topic
             * \return the found config. If none is found, a default configured one is returned.
             */
            const config::reliable_topic_config &get_reliable_topic_config(const std::string &name);

            /**
             * Helper method to add a new NearCacheConfig
             *
             * \param nearCacheConfig NearCacheConfig to be added
             * \return configured client_config for chaining
             * @see NearCacheConfig
             * 
             * Memory ownership of the config is passed to the client config
             */
            client_config &add_near_cache_config(const config::near_cache_config &near_cache_config);

            /**
             * Gets the NearCacheConfig configured for the map / cache with name
             *
             * \param name name of the map / cache
             * \return Configured NearCacheConfig
             * @see NearCacheConfig
             */
            const config::near_cache_config *get_near_cache_config(const std::string &name) const;

            /**
             * Gets {\link com.hazelcast.client.config.client_network_config}
             *
             * \return {\link com.hazelcast.client.config.client_network_config}
             * @see com.hazelcast.client.config.client_network_config
             */
            config::client_network_config &get_network_config();

            /**
             * Sets {\link com.hazelcast.client.config.client_network_config}
             *
             * \param networkConfig {\link com.hazelcast.client.config.client_network_config} to be set
             * \return configured client_config for chaining
             * @see com.hazelcast.client.config.client_network_config
             */
            client_config &set_network_config(const config::client_network_config &network_config);

            const boost::optional<std::string> &get_instance_name() const;

            client_config &set_instance_name(const std::string &instance_name);

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
             * \return configured client_config for chaining
             */
            void set_executor_pool_size(int32_t executor_pool_size);

            config::client_connection_strategy_config &get_connection_strategy_config();

            client_config &
            set_connection_strategy_config(const config::client_connection_strategy_config &connection_strategy_config);

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
            const config::client_flake_id_generator_config *find_flake_id_generator_config(const std::string &name);

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
            const config::client_flake_id_generator_config *get_flake_id_generator_config(const std::string &name);

            /**
             * Adds a flake ID generator configuration. The configuration is saved under the config
             * name, which may be a pattern with which the configuration will be
             * obtained in the future.
             *
             * \param config the flake ID configuration
             * \return this config instance
             */
            client_config &add_flake_id_generator_config(const config::client_flake_id_generator_config &config);

            /**
             *
             * \return The logger configuration.
             */
            config::logger_config &get_logger_config();

            const std::unordered_set<std::string> &get_labels() const;

            client_config &set_labels(const std::unordered_set<std::string> &labels);

            client_config &add_label(const std::string &label);

            /**
             * This feature reduces number of hops and increase performance for smart clients.
             * It is enabled by default for smart clients.
             * This config has no effect for unisocket clients.
             *
             * @param enabled enables client to get backup acknowledgements directly from the member
             *                that backups are applied
             * @return configured \ClientConfig for chaining
             */
            client_config &backup_acks_enabled(bool enabled);

            /**
             * Note that backup acks to client can be enabled only for smart client.
             * This config has no effect for unisocket clients.
             *
             * @return true if backup acknowledgements comes to client
             */
            bool backup_acks_enabled();

        private:
            friend class proxy::ReliableTopicImpl;

            const config::reliable_topic_config *lookup_reliable_topic_config(const std::string &name) const;

            std::string cluster_name_;

            config::client_network_config network_config_;

            serialization_config serialization_config_;

            boost::optional<load_balancer> load_balancer_;

            std::vector<membership_listener> membership_listeners_;

            std::vector<lifecycle_listener> lifecycle_listeners_;

            std::unordered_map<std::string, std::string> properties_;

            bool redo_operation_;

            socket_interceptor socket_interceptor_;

            std::shared_ptr<security::credentials> credentials_;

            std::unordered_map<std::string, config::reliable_topic_config> reliable_topic_config_map_;

            std::unordered_map<std::string, config::near_cache_config> near_cache_config_map_;

            boost::optional<std::string> instance_name_;

            /**
             * pool-size for internal ExecutorService which handles responses etc.
             */
            int32_t executor_pool_size_;

            config::client_connection_strategy_config connection_strategy_config_;

            std::unordered_map<std::string, config::client_flake_id_generator_config> flake_id_generator_config_map_;

            config::matcher::matching_point_config_pattern_matcher config_pattern_matcher_;

            config::logger_config logger_config_;

            std::unordered_set<std::string> labels_;

            bool backup_acks_enabled_ = true;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


