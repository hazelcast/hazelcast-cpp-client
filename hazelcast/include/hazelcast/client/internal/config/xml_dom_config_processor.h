/*
* Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <string>
#include <vector>
#include "hazelcast/util/export.h"
#include <boost/property_tree/ptree.hpp>
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/client_properties.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace config {
/**
 * Base class of the config processors working from DOM objects
 */
class HAZELCAST_API abstract_dom_config_processor
{
protected:
    /**
     * Enables strict parsing mode in which config entries are parsed as-is When disabled, dashes are ignored
     */
    bool strict;
    /**
     * Tells whether the traversed DOM is a level 3 one
     */
    bool dom_level_3;
    static bool matches(const std::string& config1, const std::string& config2);
    static hazelcast::client::serialization_config parse_serialization(
      boost::property_tree::ptree node);
    static std::string get_attribute(boost::property_tree::ptree node,
                                     const std::string& attribute);
    static bool get_bool_value(std::string value);
    static int get_integer_value(const std::string& parameter_name,
                                 const std::string& value);
    static long get_long_value(const std::string& parameter_name,
                               const std::string& value);
    static double get_double_value(const std::string& parameter_name,
                                   const std::string& value);

public:
    void static fill_properties(const boost::property_tree::ptree& node, hazelcast::client::client_config* config);
    static boost::property_tree::ptree pair_to_node(
      const std::string& node_name,
      const boost::property_tree::ptree& node_content);
    /**
     * Set collecting already seen elements. Used to detect duplicates in the configurations.
     */
    std::unordered_set<std::string> occurrence_set;
    static void fill_properties(
      const boost::property_tree::ptree& node,
      std::unordered_map<std::string, std::string>* properties,
      bool dom_level_3);
    explicit abstract_dom_config_processor(bool dom_level_3);
    abstract_dom_config_processor(bool dom_level_3, bool strict);
};

class HAZELCAST_API client_dom_config_processor
  : public abstract_dom_config_processor
{
private:
    void handle_node(const boost::property_tree::ptree& node,
                     std::string& node_name);
    void handle_security(const boost::property_tree::ptree& node) const;
    void handle_network(const boost::property_tree::ptree& node) const;
    void handle_serialization(const boost::property_tree::ptree& node) const;
    static void handle_socket_options(
      const boost::property_tree::ptree& node,
      hazelcast::client::config::client_network_config* client_network_config);
    static void handle_ssl_config(
      const boost::property_tree::ptree& node,
      hazelcast::client::config::client_network_config* client_network_config);
    static void handle_hazelcast_cloud(
      const boost::property_tree::ptree& node,
      hazelcast::client::config::client_network_config* client_network_config);
    static hazelcast::client::config::eviction_config get_eviction_config(
      const boost::property_tree::ptree& node);
    void handle_connection_strategy(const boost::property_tree::ptree& node) const;
    void handle_labels(const boost::property_tree::ptree& node) const;
    void handle_backup_ack_to_client(const boost::property_tree::ptree& node) const;
    static void handle_connection_retry(
      const boost::property_tree::ptree& node,
      hazelcast::client::config::client_connection_strategy_config*
        strategy_config);
    static void handle_aws(
      const boost::property_tree::ptree& node,
      hazelcast::client::config::client_network_config* client_network_config);
protected:
    static void handle_cluster_members(
      const boost::property_tree::ptree& node,
      hazelcast::client::config::client_network_config* client_network_config);
    void handle_flake_id_generator(const boost::property_tree::ptree& node);
    void handle_reliable_topic(const boost::property_tree::ptree& node);
    void handle_reliable_topic_node(const boost::property_tree::ptree& node) const;
    void handle_near_cache(const boost::property_tree::ptree& node);
    void handle_near_cache_node(const boost::property_tree::ptree& node) const;

public:
    static bool can_occur_multiple_times(const std::string& name);
    hazelcast::client::client_config* client_config;
    client_dom_config_processor(bool dom_level_3,
                                hazelcast::client::client_config* client_config,
                                bool strict);
    void build_config(const boost::property_tree::ptree& root_node);
};
} // namespace config
}
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
