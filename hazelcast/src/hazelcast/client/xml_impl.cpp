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
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <fstream>
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/serialization_config.h"
#include "hazelcast/client/config/ssl_config.h"
#include "hazelcast/client/config/client_flake_id_generator_config.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/client/address.h"
#include "hazelcast/client/config/client_network_config.h"
#include "hazelcast/client/config/client_aws_config.h"
#include "hazelcast/client/config/reliable_topic_config.h"
#include "hazelcast/client/config/client_connection_strategy_config.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/internal/config/xml_config_locator.h"
#include "hazelcast/client/internal/config/xml_dom_config_processor.h"
#include "hazelcast/client/internal/config/xml_variable_replacer.h"
#include "hazelcast/client/internal/config/xml_config_builder.h"
#include "hazelcast/client/hazelcast_client.h"
#include <utility>
#include <boost/property_tree/xml_parser.hpp>


namespace hazelcast {
namespace client {
namespace internal {
namespace config {
/*
client_config_sections::client_config_sections(std::string name,
                                               bool multiple_occurrence)
{
    this->name = name;
    this->multiple_occurrence = multiple_occurrence;
}
std::string
client_config_sections::get_name()
{
    return name;
}

client_config_sections client_config_sections::HAZELCAST_CLIENT(
  "hazelcast-client",
  false);
client_config_sections client_config_sections::IMPORT("import", true);
client_config_sections client_config_sections::SECURITY("security", false);
client_config_sections client_config_sections::PROXY_FACTORIES(
  "proxy-factories",
  false);
client_config_sections client_config_sections::PROPERTIES("properties", false);
client_config_sections client_config_sections::SERIALIZATION("serialization",
                                                             false);
client_config_sections client_config_sections::NATIVE_MEMORY("native-memory",
                                                             false);
client_config_sections client_config_sections::LISTENERS("listeners", false);
client_config_sections client_config_sections::NETWORK("network", false);
client_config_sections client_config_sections::LOAD_BALANCER("load-balancer",
                                                             false);
client_config_sections client_config_sections::NEAR_CACHE("near-cache", true);
client_config_sections client_config_sections::QUERY_CACHES("query-caches",
                                                            false);
client_config_sections client_config_sections::BACKUP_ACK_TO_CLIENT(
  "backup-ack-to-client-enabled",
  false);
client_config_sections client_config_sections::INSTANCE_NAME("instance-name",
                                                             false);
client_config_sections client_config_sections::CONNECTION_STRATEGY(
  "connection-strategy",
  false);
client_config_sections client_config_sections::USER_CODE_DEPLOYMENT(
  "user-code-deployment",
  false);
client_config_sections client_config_sections::FLAKE_ID_GENERATOR(
  "flake-id-generator",
  true);
client_config_sections client_config_sections::RELIABLE_TOPIC("reliable-topic",
                                                              true);
client_config_sections client_config_sections::LABELS("client-labels", false);
client_config_sections client_config_sections::CLUSTER_NAME("cluster-name",
                                                            false);
client_config_sections client_config_sections::METRICS("metrics", false);
client_config_sections client_config_sections::INSTANCE_TRACKING(
  "instance-tracking",
  false);

std::vector<client_config_sections> client_config_sections::values = {
    client_config_sections::HAZELCAST_CLIENT,
    client_config_sections::IMPORT,
    client_config_sections::SECURITY,
    client_config_sections::PROXY_FACTORIES,
    client_config_sections::PROPERTIES,
    client_config_sections::SERIALIZATION,
    client_config_sections::NATIVE_MEMORY,
    client_config_sections::LISTENERS,
    client_config_sections::NETWORK,
    client_config_sections::LOAD_BALANCER,
    client_config_sections::NEAR_CACHE,
    client_config_sections::QUERY_CACHES,
    client_config_sections::BACKUP_ACK_TO_CLIENT,
    client_config_sections::INSTANCE_NAME,
    client_config_sections::CONNECTION_STRATEGY,
    client_config_sections::USER_CODE_DEPLOYMENT,
    client_config_sections::FLAKE_ID_GENERATOR,
    client_config_sections::RELIABLE_TOPIC,
    client_config_sections::LABELS,
    client_config_sections::CLUSTER_NAME,
    client_config_sections::METRICS,
    client_config_sections::INSTANCE_TRACKING
};

bool
client_config_sections::can_occur_multiple_times(const std::string& name)
{
    for (client_config_sections section : values) {
        if (name == section.get_name()) {
            return section.multiple_occurrence;
        }
    }
    return true;
}*/

abstract_dom_config_processor::abstract_dom_config_processor(bool dom_level_3)
{
    this->dom_level_3 = dom_level_3;
    this->strict = true;
}
abstract_dom_config_processor::abstract_dom_config_processor(bool dom_level_3,
                                                             bool strict)
{
    this->dom_level_3 = dom_level_3;
    this->strict = strict;
}

bool
abstract_dom_config_processor::matches(const std::string& config1,
                                       const std::string& config2)
{
    return config1 == config2;
}
std::string
abstract_dom_config_processor::get_attribute(boost::property_tree::ptree node,
                                             const std::string& attribute)
{
    return node.get_child("<xmlattr>." + attribute).data();
}
bool
abstract_dom_config_processor::get_bool_value(std::string value)
{
    boost::algorithm::to_lower(value);
    if (value == "true") {
        return true;
    } else if (value == "false") {
        return false;
    }
    return false;
}
int
abstract_dom_config_processor::get_integer_value(
  const std::string& parameter_name,
  const std::string& value)
{
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        throw hazelcast::client::exception::invalid_configuration(
          "Invalid integer value for parameter " + parameter_name + ": " +
          value);
    }
}
int
abstract_dom_config_processor::get_integer_value(
  const std::string& parameter_name,
  const std::string& value,
  int default_value)
{
    if (value.empty()) {
        return default_value;
    }
    return get_integer_value(parameter_name, value);
}
long
abstract_dom_config_processor::get_long_value(const std::string& parameter_name,
                                              const std::string& value)
{
    try {
        return std::stol(value);
    } catch (const std::exception& e) {
        throw hazelcast::client::exception::invalid_configuration(
          "Invalid long value for parameter " + parameter_name + ": " + value);
    }
}
long
abstract_dom_config_processor::get_long_value(const std::string& parameter_name,
                                              const std::string& value,
                                              long default_value)
{
    if (value.empty()) {
        return default_value;
    }
    return get_long_value(parameter_name, value);
}
double
abstract_dom_config_processor::get_double_value(
  const std::string& parameter_name,
  const std::string& value)
{
    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        throw hazelcast::client::exception::invalid_configuration(
          "Invalid double value for parameter " + parameter_name + ": " +
          value);
    }
}
double
abstract_dom_config_processor::get_double_value(
  const std::string& parameter_name,
  const std::string& value,
  double default_value)
{
    if (value.empty()) {
        return default_value;
    }
    return get_double_value(parameter_name, value);
}

hazelcast::client::serialization_config
abstract_dom_config_processor::parse_serialization(
  boost::property_tree::ptree node)
{
    auto serialization_config = new hazelcast::client::serialization_config();
    for (auto& child : node) {
        std::string name = child.first;
        if (matches("portable-version", name)) {
            std::string value = child.second.data();
            serialization_config->set_portable_version(
              get_integer_value(child.first, value));
        } else if (matches("check-class-def-errors", name)) {
            std::string value = child.second.data();
            // serializationConfig.setCheckClassDefErrors(getBooleanValue(value));
        } else if (matches("use-native-byte-order", name)) {
            serialization_config->set_byte_order(boost::endian::order::native);
        } else if (matches("byte-order", name)) {
            std::string value = child.second.data();
            if (matches("BIG_ENDIAN", value)) {
                serialization_config->set_byte_order(boost::endian::order::big);
            } else if (matches("LITTLE_ENDIAN", value)) {
                serialization_config->set_byte_order(
                  boost::endian::order::little);
            }
        } /*else if (matches("enable-compression", name)) {
            serializationConfig.setEnableCompression(getBooleanValue(getTextContent(child)));
        } else if (matches("enable-shared-object", name)) {
            serializationConfig.setEnableSharedObject(getBooleanValue(getTextContent(child)));
        } else if (matches("allow-unsafe", name)) {
            serializationConfig.setAllowUnsafe(getBooleanValue(getTextContent(child)));
        } else if (matches("allow-override-default-serializers", name)) {
            serializationConfig.setAllowOverrideDefaultSerializers(getBooleanValue(getTextContent(child)));
        } else if (matches("data-serializable-factories", name)) {
            fillDataSerializableFactories(child, serializationConfig);
        } else if (matches("portable-factories", name)) {
            fillPortableFactories(child, serializationConfig);
        } else if (matches("serializers", name)) {
            fillSerializers(child, serializationConfig);
        } else if (matches("java-serialization-filter", name)) {
            fillJavaSerializationFilter(child, serializationConfig);
        } else if (matches("compact-serialization", name)) {
            handleCompactSerialization(child, serializationConfig);
        }*/
    }
    return *serialization_config;
}
/*
void
abstract_dom_config_processor::fill_properties(
  const boost::property_tree::ptree& node,
  const hazelcast::client::client_properties& properties)
{
    fill_properties(node, properties, dom_level_3);
}*//*
void
abstract_dom_config_processor::fill_properties(
  const boost::property_tree::ptree& node,
  const hazelcast::client::client_properties& properties,
  bool dom_level_3)
{
    for (auto& pair : node){
        if(pair.first == "<xmlcomment>"){
            continue;
        }
        std::string property_name =  get_attribute(pair.second,"name");
        std::string value = pair.second.data();

    }
}*/
void
abstract_dom_config_processor::fill_properties(
  const boost::property_tree::ptree& node,
  std::unordered_map<std::string, std::string>* properties,
  bool dom_level_3)
{
    for (auto& pair : node){
        if(pair.first == "<xmlcomment>"){
            continue;
        }
        std::string property_name =  get_attribute(pair.second,"name");
        std::string value = pair.second.data();
        properties->emplace(property_name, value);

    }
}

void
abstract_dom_config_processor::fill_properties(
  const boost::property_tree::ptree& node,
  hazelcast::client::client_config* config
  )
{
    for (auto& pair : node){
        if(pair.first == "<xmlcomment>"){
            continue;
        }
        std::string property_name =  get_attribute(pair.second,"name");
        std::string value = pair.second.data();
        config->set_property(property_name, value);

    }
}

boost::property_tree::ptree
abstract_dom_config_processor::pair_to_node(
  const std::string& node_name,
  const boost::property_tree::ptree& node_content)
{
    boost::property_tree::ptree temp;
    temp.add_child(node_name, node_content);
    return temp.get_child(node_name);
}

client_dom_config_processor::client_dom_config_processor(
  bool dom_level_3,
  hazelcast::client::client_config* client_config)
  : abstract_dom_config_processor(dom_level_3)
{
    this->client_config = client_config;
}
client_dom_config_processor::client_dom_config_processor(
  bool dom_level_3,
  hazelcast::client::client_config* client_config,
  bool strict)
  : abstract_dom_config_processor(dom_level_3, strict)
{
    this->client_config = client_config;
}
bool client_dom_config_processor::can_occur_multiple_times(const std::string& name){
    if(matches(name, "import") || matches(name ,"flake-id-generator" )  || matches(name, "reliable-topic") || matches(name, "near-cache") ){
        return true;
    }
    return false;
}
void
client_dom_config_processor::build_config(
  const boost::property_tree::ptree& root_node)
{
    for (auto& pair : root_node) {
        std::string node_name = pair.first;
        if (node_name == "<xmlattr>") {
            continue;
        }
        if (node_name == "<xmlcomment>") {
            continue;
        }
        auto node = pair_to_node(pair.first, pair.second);
        if (occurrence_set->find(node_name) == occurrence_set->end()) {
            handle_node(node, node_name);
        } else {
            throw hazelcast::client::exception::invalid_configuration(
              "Duplicate '" + node_name +
              "' definition found in the configuration");
        }
        if (!can_occur_multiple_times(node_name)) {
            occurrence_set->insert(node_name);
        }
    }
}
void
client_dom_config_processor::handle_node(
  const boost::property_tree::ptree& node,
  std::string& node_name)
{
    if (matches(node_name, "security")) {
        handle_security(node);
    } else if (matches(node_name,"proxy-factories")) {
        handle_proxy_factories(node);
    } else if (matches(node_name,
                       "properties")) {
        fill_properties(node, client_config);
    } else if (matches(node_name,
                       "serialization")) {
        handle_serialization(node);
    } else if (matches(node_name,
                       "native-memory")) { // not supported

    } else if (matches(node_name,
                       "listeners")) {
        handle_listeners(node);
    } else if (matches(node_name, "network")) {
        handle_network(node);
    } else if (matches(node_name,
                       "load-balancer")) {
        handle_load_balancer(node);
    } else if (matches(node_name,
                       "near-cache")) {
        handle_near_cache(node);
    } else if (matches(node_name,"query-cache")) { // not supported

    } else if (matches(node_name,
                       "instance-name")) {
        client_config->set_instance_name(node.data());
    } else if (matches(
                 node_name,
                 "connection-strategy")) {
        handle_connection_strategy(node);
    } else if (matches(node_name,
                       "user-code-deployment")) { // not supported

    } else if (matches(node_name,
                       "flake-id-generator")) {
        handle_flake_id_generator(node);
    } else if (matches(node_name,
                       "reliable-topic")) {
        handle_reliable_topic(node);
    } else if (matches(node_name, "client-labels")) {
        handle_labels(node);
    } else if (matches(
                 node_name,
                 "backup-ack-to-client-enabled")) {
        handle_backup_ack_to_client(node);
    } else if (matches(node_name,
                       "cluster-name")) {
        client_config->set_cluster_name(node.data());
    } else if (matches(node_name,
                       "metrics")) { //?????

    } else if (matches(node_name,"instance-tracking")) {//?????

    }
}

void
client_dom_config_processor::handle_security(
  const boost::property_tree::ptree& node)
{
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("username-password", node_name)) {
            client_config->set_credentials(
              std::make_shared<
                hazelcast::client::security::username_password_credentials>(
                get_attribute(child, "username"),
                get_attribute(child, "password")));
        } else if (matches("token", node_name)) {
            // TODO
        } else if (matches("credentials-factory", node_name)) { // not supported

        } else if (matches("kerberos", node_name)) { // not supported

        } else if (matches("realms", node_name)) {
            handle_realms(child);
        }
    }
}
void
client_dom_config_processor::handle_serialization(
  const boost::property_tree::ptree& node)
{
    hazelcast::client::serialization_config serialization_config =
      parse_serialization(node);
    client_config->set_serialization_config(serialization_config);
}
void
client_dom_config_processor::handle_network(
  const boost::property_tree::ptree& node)
{
    auto client_network_config =
      new hazelcast::client::config::client_network_config();
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("cluster-members", node_name)) {
            handle_cluster_members(child, client_network_config);
        } else if (matches("smart-routing", node_name)) {
            client_network_config->set_smart_routing(
              get_bool_value(pair.second.data()));
        } else if (matches("redo-operation", node_name)) {
            client_config->set_redo_operation(
              get_bool_value(pair.second.data()));
        } else if (matches("connection-timeout", node_name)) {
            client_network_config->set_connection_timeout(
              std::chrono::milliseconds(
                get_integer_value(node_name, pair.second.data())));
        } else if (matches("socket-options", node_name)) {
            handle_socket_options(child, client_network_config);
        } else if (matches("socket-interceptor", node_name)) { // not supported

        } else if (matches("ssl", node_name)) {
            handle_ssl_config(child, client_network_config);
        } // else if (AliasedDiscoveryConfigUtils.supports(node_name)) {//not supported
          //  handleAliasedDiscoveryStrategy(child, clientNetworkConfig, nodeName);}
        else if (matches("discovery-strategies", node_name)) { // not supported

        } else if (matches("auto-detection", node_name)) { // not supported

        } else if (matches("outbound-ports", node_name)) { // not supported

        } else if (matches("icmp-ping", node_name)) { // not supported

        } else if (matches("hazelcast-cloud", node_name)) {
            handle_hazelcast_cloud(child, client_network_config);
        }
    }
    client_config->set_network_config(*client_network_config);
}

void
client_dom_config_processor::handle_cluster_members(
  const boost::property_tree::ptree& node,
  hazelcast::client::config::client_network_config* client_network_config)
{
    for (auto& pair : node) {
        if (matches("address", pair.first)) {
            int port = 5701; // port is not included in the example usage in hazelcast-client-full-example.xml
            client_network_config->add_address(
              hazelcast::client::address(pair.second.data(), port));
        }
    }
}

void
client_dom_config_processor::handle_socket_options(
  const boost::property_tree::ptree& node,
  hazelcast::client::config::client_network_config* client_network_config)
{
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("tcp-no-delay", node_name)) {
            client_network_config->get_socket_options().set_tcp_no_delay(
              get_bool_value(pair.second.data()));
        } else if (matches("keep-alive", node_name)) {
            client_network_config->get_socket_options().set_keep_alive(
              get_bool_value(pair.second.data()));
        } else if (matches("reuse-address", node_name)) {
            client_network_config->get_socket_options().set_reuse_address(
              get_bool_value(pair.second.data()));
        } else if (matches("linger-seconds", node_name)) {
            client_network_config->get_socket_options().set_linger_seconds(
              get_integer_value(pair.first, pair.second.data()));
        } else if (matches("buffer-size", node_name)) {
            client_network_config->get_socket_options()
              .set_buffer_size_in_bytes(
                get_integer_value(pair.first, pair.second.data()));
        }
    }
}
void
client_dom_config_processor::handle_ssl_config(
  const boost::property_tree::ptree& node,
  hazelcast::client::config::client_network_config* client_network_config)
{
}
void
client_dom_config_processor::handle_hazelcast_cloud(
  const boost::property_tree::ptree& node,
  hazelcast::client::config::client_network_config* client_network_config)
{
    bool enabled = false;
    try {
        std::string enable = get_attribute(node, "enabled");
        boost::algorithm::trim(enable);
        enabled = get_bool_value(enable);
    } catch (const boost::exception& e) {
    }
    client_network_config->get_cloud_config().enabled = enabled;
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("discovery-token", pair.first)) {
            client_network_config->get_cloud_config().discovery_token =
              pair.second.data();
        }
    }
}
void
client_dom_config_processor::client_dom_config_processor::
  handle_flake_id_generator(const boost::property_tree::ptree& node)
{
    handle_flake_id_generator_node(node);
}

void
client_dom_config_processor::client_dom_config_processor::
  handle_flake_id_generator_node(const boost::property_tree::ptree& node)
{
    std::string name = get_attribute(node, "name");
    auto config =
      new hazelcast::client::config::client_flake_id_generator_config(name);
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        boost::algorithm::to_lower(node_name);
        if (matches("prefetch-count", node_name)) {
            config->set_prefetch_count(
              get_integer_value(pair.first, pair.second.data()));
        } else if (matches("prefetch-validity-millis", node_name)) {
            config->set_prefetch_validity_duration(std::chrono::milliseconds(
              get_long_value(node_name, pair.second.data())));
        }
    }
    client_config->add_flake_id_generator_config(*config);
}

void
client_dom_config_processor::handle_reliable_topic(
  const boost::property_tree::ptree& node)
{
    handle_reliable_topic_node(node);
}
void
client_dom_config_processor::handle_reliable_topic_node(
  const boost::property_tree::ptree& node)
{
    std::string name = get_attribute(node, "name");
    auto config = new hazelcast::client::config::reliable_topic_config(name);
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("topic-overload-policy", node_name)) { // doesnt exist

        } else if (matches("read-batch-size", node_name)) {
            config->set_read_batch_size(
              get_integer_value(pair.first, pair.second.data()));
        }
    }
    client_config->add_reliable_topic_config(*config);
}
void
client_dom_config_processor::handle_near_cache(
  const boost::property_tree::ptree& node)
{
    handle_near_cache_node(node);
}
void
client_dom_config_processor::handle_near_cache_node(
  const boost::property_tree::ptree& node)
{
    hazelcast::client::config::near_cache_config* near_cache_config;
    try {
        std::string name = get_attribute(node, "name");
        near_cache_config =
          new hazelcast::client::config::near_cache_config(name);
    } catch (const boost::exception& e) {
        near_cache_config =
          new hazelcast::client::config::near_cache_config("default");
    }
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("time-to-live-seconds", node_name)) {
            near_cache_config->set_time_to_live_seconds(
              get_integer_value(pair.first, pair.second.data()));
        } else if (matches("max-idle-seconds", node_name)) {
            near_cache_config->set_max_idle_seconds(
              get_integer_value(pair.first, pair.second.data()));
        } else if (matches("in-memory-format", node_name)) { // NO NATIVE ??
            if (pair.second.data() == "BINARY") {
                near_cache_config->set_in_memory_format(
                  hazelcast::client::config::in_memory_format::BINARY);
            } else if (pair.second.data() == "OBJECT") {
                near_cache_config->set_in_memory_format(
                  hazelcast::client::config::in_memory_format::OBJECT);
            }
        } else if (matches("serialize-keys", node_name)) { //??
            // serializeKeys = Boolean.parseBoolean(getTextContent(child));
            // nearCacheConfig.setSerializeKeys(serializeKeys);
        } else if (matches("invalidate-on-change", node_name)) {
            near_cache_config->set_invalidate_on_change(
              get_bool_value(pair.second.data()));
        } else if (matches("local-update-policy", node_name)) {
            if (pair.second.data() == "CACHE") {
                near_cache_config->set_local_update_policy(
                  hazelcast::client::config::near_cache_config::CACHE);
            } else if (pair.second.data() == "INVALIDATE") {
                near_cache_config->set_local_update_policy(
                  hazelcast::client::config::near_cache_config::INVALIDATE);
            }
        } else if (matches("eviction", node_name)) {
            near_cache_config->set_eviction_config(
              get_eviction_config(pair.second));
        } else if (
          matches(
            "preloader",
            node_name)) { //??
                          // nearCacheConfig.setPreloaderConfig(getNearCachePreloaderConfig(child));
        }
    }
    /*if (serializeKeys != null && !serializeKeys &&
    nearCacheConfig.getInMemoryFormat() == InMemoryFormat.NATIVE) { std::cout <<
    "WARNING: " << "The Near Cache doesn't support keys by-reference with NATIVE
    in-memory-format. This setting will have no effect!" << std::endl;
    }*/
    client_config->add_near_cache_config(*near_cache_config);
}
hazelcast::client::config::eviction_config
client_dom_config_processor::get_eviction_config(
  const boost::property_tree::ptree& node)
{
    auto eviction_config = hazelcast::client::config::eviction_config();
    try {
        auto attr = get_attribute(node, "size");
        eviction_config.set_size(get_integer_value("size", attr));
    } catch (const boost::exception& e) {
    }
    try {
        auto attr = get_attribute(node, "max-size-policy");
        if (attr == "ENTRY_COUNT") { // other policies are not implemented
            eviction_config.set_maximum_size_policy(
              hazelcast::client::config::eviction_config::ENTRY_COUNT);
        }
    } catch (const boost::exception& e) {
    }
    try {
        auto attr = get_attribute(node, "eviction-policy");
        if (attr == "LRU") {
            eviction_config.set_eviction_policy(
              hazelcast::client::config::eviction_policy::LRU);
        } else if (attr == "LFU") {
            eviction_config.set_eviction_policy(
              hazelcast::client::config::eviction_policy::LFU);
        } else if (attr == "NONE") {
            eviction_config.set_eviction_policy(
              hazelcast::client::config::eviction_policy::NONE);
        } else if (attr == "RANDOM") {
            eviction_config.set_eviction_policy(
              hazelcast::client::config::eviction_policy::RANDOM);
        }
    } catch (const boost::exception& e) {
    }
    try {
        get_attribute(node,
                      "comparator-class-name"); // couldn't find this option
    } catch (const boost::exception& e) {
    }
    return eviction_config;
}
void
client_dom_config_processor::handle_connection_strategy(
  const boost::property_tree::ptree& node)
{
    auto strategy_config =
      hazelcast::client::config::client_connection_strategy_config();
    try {
        std::string attr_val = get_attribute(node, "async-start");
        boost::algorithm::trim(attr_val);
        strategy_config.set_async_start(get_bool_value(attr_val));
    } catch (const boost::exception& e) {
        strategy_config.set_async_start(false);
    }
    try {
        std::string attr_val1 = get_attribute(node, "reconnect-mode");
        boost::algorithm::trim(attr_val1);
        if (attr_val1 == "ON") {
            strategy_config.set_reconnect_mode(
              hazelcast::client::config::client_connection_strategy_config::
                reconnect_mode::ON);
        } else if (attr_val1 == "OFF") {
            strategy_config.set_reconnect_mode(
              hazelcast::client::config::client_connection_strategy_config::
                reconnect_mode::OFF);
        } else if (attr_val1 == "ASYNC") {
            strategy_config.set_reconnect_mode(
              hazelcast::client::config::client_connection_strategy_config::
                reconnect_mode::ASYNC);
        }
    } catch (const boost::exception& e) {
    }
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("connection-retry", node_name)) {
            handle_connection_retry(child, strategy_config);
        }
    }
    client_config->set_connection_strategy_config(strategy_config);
}
void
client_dom_config_processor::handle_connection_retry(
  const boost::property_tree::ptree& node,
  hazelcast::client::config::client_connection_strategy_config strategy_config)
{
    auto connection_retry_config =
      new hazelcast::client::config::connection_retry_config();
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("initial-backoff-millis", node_name)) {
            connection_retry_config->set_initial_backoff_duration(
              std::chrono::milliseconds(
                get_integer_value(node_name, pair.second.data())));
        } else if (matches("max-backoff-millis", node_name)) {
            connection_retry_config->set_max_backoff_duration(
              std::chrono::milliseconds(
                get_integer_value(node_name, pair.second.data())));
        } else if (matches("multiplier", node_name)) {
            connection_retry_config->set_multiplier(
              get_double_value(node_name, pair.second.data()));
        } else if (matches("cluster-connect-timeout-millis", node_name)) {
            connection_retry_config->set_cluster_connect_timeout(
              std::chrono::milliseconds(
                get_long_value(node_name, pair.second.data())));
        } else if (matches("jitter", node_name)) {
            connection_retry_config->set_jitter(
              get_double_value(node_name, pair.second.data()));
        }
    }
    strategy_config.set_retry_config(*connection_retry_config);
}

void
client_dom_config_processor::handle_labels(
  const boost::property_tree::ptree& node)
{
    for (auto& child : node) {
        client_config->add_label(child.first);
    }
}
void
client_dom_config_processor::handle_proxy_factories(
  const boost::property_tree::ptree& node)
{
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        handle_proxy_factories_node(child);
    }
}
void
client_dom_config_processor::handle_proxy_factories_node(
  const boost::property_tree::ptree& node)
{
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches(node_name,
                    "proxy-factory")) { // not sure if it is supported
        }
    }
}
void
client_dom_config_processor::handle_listeners(
  const boost::property_tree::ptree& node)
{
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        if (matches(pair.first, "listener")) { // not sure how to add listeners
        }
    }
}
void
client_dom_config_processor::handle_backup_ack_to_client(
  const boost::property_tree::ptree& node)
{
    client_config->backup_acks_enabled(get_bool_value(node.data()));
}
void
client_dom_config_processor::handle_realms(
  const boost::property_tree::ptree& node)
{
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        if (matches("realm", pair.first)) {
            handle_realm(child);
        }
    }
}
void
client_dom_config_processor::handle_realm(
  const boost::property_tree::ptree& node)
{
    try {
        auto realm_name = get_attribute(node, "name");
    } catch (const boost::exception& e) {
    }
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("authentication",
                    node_name)) { // authentication not supported
        }
    }
}
void
client_dom_config_processor::handle_load_balancer(
  const boost::property_tree::ptree& node)
{ // not sure how to configure load_balancer
    /*
    std::string type = get_attribute(node,"type");
    if (matches("random", type)) {
        clientConfig.setLoadBalancer(new RandomLB());
    } else if (matches("round-robin", type)) {
        clientConfig.setLoadBalancer(new RoundRobinLB());
    } else if (matches("custom", type)) {
        String loadBalancerClassName = parseCustomLoadBalancerClassName(node);
        clientConfig.setLoadBalancerClassName(loadBalancerClassName);
    }*/
}

const std::string declarative_config_util::SYSPROP_MEMBER_CONFIG = "hazelcast.config";
const std::string declarative_config_util::SYSPROP_CLIENT_CONFIG =
  "hazelcast.client.config";
const std::string declarative_config_util::SYSPROP_CLIENT_FAILOVER_CONFIG =
  "hazelcast.client.failover.config";
const std::vector<std::string> declarative_config_util::XML_ACCEPTED_SUFFIXES = {
    "xml"
};
const std::string declarative_config_util::XML_ACCEPTED_SUFFIXES_STRING = "[xml]";
const std::vector<std::string> declarative_config_util::YAML_ACCEPTED_SUFFIXES = {
    "yaml",
    "yml"
};
const std::string declarative_config_util::YAML_ACCEPTED_SUFFIXES_STRING =
  "[yaml, yml]";
const std::vector<std::string> declarative_config_util::ALL_ACCEPTED_SUFFIXES = {
    "xml",
    "yaml",
    "yml"
};
const std::string declarative_config_util::ALL_ACCEPTED_SUFFIXES_STRING =
  "[xml, yaml, yml]";

void
declarative_config_util::validate_suffix_in_system_property(
  const std::string& property_key)
{
    std::string config_system_property =
      ""; // System.getProperty(propertyKey); TODO
    if (config_system_property.empty()) {
        return;
    }
    if (!is_accepted_suffix_configured(config_system_property,
                                       ALL_ACCEPTED_SUFFIXES)) {
        throw_unaccepted_suffix_in_system_property(
          property_key, config_system_property, ALL_ACCEPTED_SUFFIXES);
    }
}
void
declarative_config_util::throw_unaccepted_suffix_in_system_property(
  const std::string& property_key,
  const std::string& config_resource,
  const std::vector<std::string>& accepted_suffixes)
{

    std::string message =
      "The suffix of the resource \'" + config_resource +
      "\' referenced in \'" + property_key +
      "\' is not in the list of accepted " + "suffixes: \'[" +
      boost::algorithm::join(accepted_suffixes, ", ") + "]\'";
    throw hazelcast::client::exception::hazelcast_(message);
}
bool
declarative_config_util::is_accepted_suffix_configured(
  const std::string& config_file,
  std::vector<std::string> accepted_suffixes)
{
    std::string config_file_lower;
    if (config_file.empty()) {
        config_file_lower = config_file;
    } else {
        config_file_lower = config_file;
        boost::algorithm::to_lower(config_file_lower);
    }
    int last_dot_index = (int)config_file_lower.find_last_of('.');
    if (last_dot_index == -1) {
        return false;
    }
    std::string config_file_suffix =
      config_file_lower.substr(last_dot_index + 1);
    return std::find(accepted_suffixes.begin(),
                     accepted_suffixes.end(),
                     config_file_suffix) != accepted_suffixes.end();
}

std::ifstream*
abstract_config_locator::get_in()
{
    return in;
}

/*
bool abstract_config_locator::locate_everywhere(){
    return locate_from_system_property_or_fail_on_unaccepted_suffix()
           || locate_in_work_directory()
           || locate_default();
}*/
bool
abstract_config_locator::load_from_working_directory(
  const std::string& config_file_path)
{
    try {
        std::FILE* file;
        file = fopen(config_file_path.c_str(), "r");
        if (file == nullptr) {
            std::cout << "FINEST: "
                      << "Could not find " + config_file_path +
                           " in the working directory."
                      << std::endl;
            return false;
        }
        std::cout << "INFO: Loading " + config_file_path +
                       " from the working directory."
                  << std::endl;
        configuration_file = file;
        in = new std::ifstream();
        in->open(config_file_path, std::ios::in);
        if (in->fail()) {
            throw hazelcast::client::exception::hazelcast_(
              "Failed to open file: " + config_file_path);
        }
        return true;
    } catch (const std::runtime_error& e) {
        throw hazelcast::client::exception::hazelcast_(e.what());
    }
}

bool
abstract_config_locator::load_from_working_directory(
  const std::string& config_file_prefix,
  const std::vector<std::string>& accepted_suffixes)
{
    if (accepted_suffixes.empty()) {
        throw std::invalid_argument(
          "Parameter acceptedSuffixes must not be empty");
    }
    for (const auto& suffix : accepted_suffixes) {
        if (suffix.empty()) {
            throw std::invalid_argument(
              "Parameter acceptedSuffixes must not contain empty strings");
        }
        if (load_from_working_directory(config_file_prefix + "." + suffix)) {
            return true;
        }
    }
    return false;
}
bool
abstract_config_locator::load_from_system_property(
  const std::string& property_key,
  const std::vector<std::string>& accepted_suffixes)
{
    return load_from_system_property(property_key, false, accepted_suffixes);
}
void
abstract_config_locator::load_system_property_file_resource(
  const std::string& config_system_property)
{
    configuration_file = fopen(config_system_property.c_str(), "r");
    std::cout << "Using configuration file at " << config_system_property
              << std::endl;
    if (configuration_file == nullptr) {
        std::string msg =
          "Config file at " + config_system_property + " doesn't exist.";
        throw hazelcast::client::exception::hazelcast_(msg);
    }
    in->open(config_system_property);
    if (in->fail()) {
        throw hazelcast::client::exception::hazelcast_("Failed to open file: " +
                                                       config_system_property);
    }
}

bool
abstract_config_locator::load_from_system_property(
  const std::string& property_key,
  bool fail_on_unaccepted_suffix,
  const std::vector<std::string>& accepted_suffixes)
{
    if (accepted_suffixes.empty()) {
        throw hazelcast::client::exception::illegal_argument(
          "Parameter acceptedSuffixes must not be empty");
    }
    try {
        std::string config_system_property =
          "temp"; // System.getProperty(propertyKey); TODO

        if (config_system_property.empty()) {
            std::cout << "FINEST: "
                      << "Could not find " + property_key + " System property";
            return false;
        }

        if (!declarative_config_util::is_accepted_suffix_configured(
              config_system_property, accepted_suffixes)) {
            if (fail_on_unaccepted_suffix) {
                declarative_config_util::
                  throw_unaccepted_suffix_in_system_property(
                    property_key, config_system_property, accepted_suffixes);
            } else {
                return false;
            }
        }

        std::cout << "INFO: "
                  << "Loading configuration " + config_system_property +
                       " from System property"
                  << property_key << std::endl;
        load_system_property_file_resource(config_system_property);
        return true;
    } catch (const hazelcast::client::exception::hazelcast_& e) {
        throw hazelcast::client::exception::hazelcast_(e.what());
    } catch (const std::runtime_error& e) {
        throw hazelcast::client::exception::hazelcast_(e.what());
    }
}
bool
abstract_config_locator::load_from_system_property_or_fail_on_unaccepted_suffix(
  const std::string& property_key,
  const std::vector<std::string>& accepted_suffixes)
{
    return load_from_system_property(property_key, true, accepted_suffixes);
}
bool
abstract_config_locator::is_config_present()
{
    return in != nullptr || configuration_file != nullptr;
}
bool
xml_client_config_locator::
  locate_from_system_property_or_fail_on_unaccepted_suffix()
{
    return load_from_system_property_or_fail_on_unaccepted_suffix(
      declarative_config_util::SYSPROP_CLIENT_CONFIG,
      declarative_config_util::XML_ACCEPTED_SUFFIXES);
}
bool
xml_client_config_locator::locate_in_work_directory()
{
    return load_from_working_directory("hazelcast-client.xml");
}
bool
xml_client_config_locator::locate_from_system_property()
{
    return load_from_system_property(
      declarative_config_util::SYSPROP_CLIENT_CONFIG,
      declarative_config_util::XML_ACCEPTED_SUFFIXES);
}


const hazelcast::client::client_property* abstract_config_builder::VALIDATION_ENABLED_PROP =
  new hazelcast::client::client_property(
    "hazelcast.config.schema.validation.enabled",
    "true");

bool
abstract_config_builder::should_validate_the_schema()
{
    return true; // TODO
}

std::string
abstract_xml_config_helper::get_release_version()
{
    return hazelcast::client::version().to_string();
}
void
abstract_xml_config_helper::schema_validation(boost::property_tree::ptree doc)
{
    // TODO property_tree doesn't support validation
}

std::string
abstract_xml_config_builder::get_attribute(boost::property_tree::ptree node,
                                           const std::string& att_name)
{
    return node.get_child("<xmlattr>." + att_name).data();
}

void
abstract_xml_config_builder::process(boost::property_tree::ptree* root)
{
    replace_imports(root);
    replace_variables(root);
}

void
abstract_xml_config_builder::replace_variables(
  boost::property_tree::ptree* root)
{
    bool fail_fast = false;
    auto replacers = new std::vector<property_replacer>();
    boost::property_tree::ptree node;
    try {
        node = root->get_child("config-replacers");
        try {
            std::string fail_fast_attr =
              get_attribute(node, "fail-if-value-missing");
            if (fail_fast_attr == "true") {
                fail_fast = true;
            }
            for (auto& n : node) {
                if(n.first == "<xmlattr>"){
                    continue ;
                }
                std::string value = n.first;
                if ("replacer" == value) {

                    replacers->push_back(create_replacer(n.second));
                }
            }
        } catch (const boost::exception& e) {

        }
    } catch (const boost::exception& e) {

    }
    config_replacer_helper::traverse_children_and_replace_variables(
      root, *replacers, fail_fast, *(new xml_dom_variable_replacer()));
}
void
abstract_xml_config_builder::replace_imports(boost::property_tree::ptree* root)
{
    replace_variables(root);

    // there is something to give error here

    for (auto& child : *root) {
        if (child.first == "import") {
            boost::property_tree::ptree temp;
            temp.add_child(child.first, child.second);
            std::string resource =
              get_attribute(temp.get_child(child.first), "resource");
            std::ifstream* stream = new std::ifstream();
            stream->open(resource);
            if (stream->fail()) {
                throw hazelcast::client::exception::invalid_configuration(
                  "Failed to load resource: " + resource);
            }
            if (!currently_imported_files->emplace(resource).second) {
                throw hazelcast::client::exception::invalid_configuration(
                  "Resource '" + resource +
                  "' is already loaded! This can be due to" +
                  " duplicate or cyclic imports.");
            }
            boost::property_tree::ptree imported_root =
              parse(stream).get_child("hazelcast-client");
            replace_imports(&imported_root);
            for (auto& imported_node : imported_root) {
                if (imported_node.first == "<xmlattr>") {
                    continue;
                }
                root->put_child(imported_node.first, imported_node.second);
            }
            for (auto it = root->begin(); it != root->end(); it++) {
                if (it->first == "import") {
                    root->erase(it);
                    break;
                }
            }
        }
    }
}

property_replacer
abstract_xml_config_builder::create_replacer(
  const boost::property_tree::ptree& node)
{
    std::string replacer_class = get_attribute(node, "class-name");
    std::unordered_map<std::string, std::string>* properties_  = new std::unordered_map<std::string, std::string>();
    for (auto& n : node) {
        std::string value = n.first;
        if ("properties" == value) {
            fill_properties(n.second, properties_);
        }
    }
    property_replacer replacer;
    replacer.init(properties_);
    return replacer;
}

std::string
abstract_xml_config_helper::get_namespace_type()
{
    return "client-config";
}
abstract_xml_config_builder::abstract_xml_config_builder() = default;

void
abstract_xml_config_builder::fill_properties(
  const boost::property_tree::ptree& node,
  std::unordered_map<std::string, std::string>* properties_)
{
    abstract_dom_config_processor::fill_properties(
      node, properties_, dom_level_3);
}
void
abstract_xml_config_builder::set_properties_internal(
  std::unordered_map<std::string, std::string> properties_)
{
    this->properties = &properties_;
}
std::unordered_map<std::string, std::string>*
abstract_xml_config_builder::get_properties()
{
    return properties;
}

xml_client_config_builder::xml_client_config_builder(
  const std::string& resource)
{
    std::ifstream stream(resource, std::ifstream::in);
    this->in = &stream;
}

xml_client_config_builder::xml_client_config_builder(std::ifstream* in)
{
    this->in = in;
}

xml_client_config_builder::xml_client_config_builder()
  : xml_client_config_builder(new xml_client_config_locator())
{
}

xml_client_config_builder::xml_client_config_builder(
  xml_client_config_locator* locator)
{
    this->in = locator->get_in();
}
boost::property_tree::ptree
xml_client_config_builder::parse(std::ifstream* input_stream)
{
    boost::property_tree::ptree tree;
    try {
        boost::property_tree::read_xml(*input_stream, tree);
        input_stream->close();
        return tree;
    } catch (const boost::exception& e) {
        std::string msg = "Failed to parse Config Stream     HazelcastClient startup interrupted."; // TODO
        input_stream->close();
        throw hazelcast::client::exception::invalid_configuration(msg);
    }
}
xml_client_config_builder
xml_client_config_builder::set_properties(
  std::unordered_map<std::string, std::string> properties)
{
    set_properties_internal(std::move(properties));
    return *this;
}
hazelcast::client::client_config
xml_client_config_builder::build()
{
    hazelcast::client::client_config client_config;
    parse_and_build_config(&client_config);
    in->close();
    return client_config;
}

void
xml_client_config_builder::parse_and_build_config(
  hazelcast::client::client_config* client_config)
{
    auto root = parse(in);
    try {
        root = root.get_child("hazelcast-client");
    } catch (const boost::exception& e) {
        throw hazelcast::client::exception::invalid_configuration(
          "Invalid root element in xml configuration! Expected: <hazelcast-client>");
    }
    try {
        root.data();
    } catch (const boost::exception& e) {
        dom_level_3 = false;
    }
    process(&root);
    if (should_validate_the_schema()) {
        schema_validation(root);
    }
    client_dom_config_processor x(dom_level_3, client_config, false);
    x.build_config(root);
}

property_replacer::property_replacer() = default;

void
property_replacer::init(std::unordered_map<std::string, std::string>* properties_)
{
    this->properties = properties_;
}
std::string
property_replacer::get_prefix()
{
    return "";
}

std::string
property_replacer::get_replacement(const std::string& variable)
{
    return properties->at(variable);
}

config_replacer_helper::config_replacer_helper() = default;

void
config_replacer_helper::traverse_children_and_replace_variables(
  boost::property_tree::ptree* root,
  const property_replacer& replacer,
  bool fail_fast,
  xml_dom_variable_replacer variable_replacer)
{
    try {
        try{
            auto attributes = root->get_child("<xmlattr>");
            for (auto& attribute : attributes) {
                // auto attr = pair_to_node(attribute.first,attribute.second);
                variable_replacer.replace_variables(
                  &attribute.second, replacer, fail_fast);
            }
        } catch (const boost::exception& e) {

        }

        if (!root->data().empty()) {
            variable_replacer.replace_variables(root, replacer, fail_fast);
        }
        for (auto& pair : *root) {
            // auto child = pair_to_node(pair.first,pair.second);
            if(pair.first == "<xmlcomment>"){
                continue ;
            }
            traverse_children_and_replace_variables(
              &pair.second, replacer, fail_fast, variable_replacer);
        }
    } catch (const boost::exception& e) {
    }
}
void
config_replacer_helper::traverse_children_and_replace_variables(
  boost::property_tree::ptree* root,
  const std::vector<property_replacer>& replacers,
  bool fail_fast,
  const xml_dom_variable_replacer& variable_replacer)
{
    for (const property_replacer& replacer : replacers) {
        traverse_children_and_replace_variables(
          root, replacer, fail_fast, variable_replacer);
    }
}

void
abstract_dom_variable_replacer::handle_missing_variable(
  const std::string& variable,
  const std::string& node_name,
  bool fail_fast)
{
    std::string message =
      "Could not find a replacement for " + variable + " on node " + node_name;
    if (fail_fast) {
        throw hazelcast::client::exception::invalid_configuration(message);
    }
    std::cout << "WARNING: " << message << std::endl;
}
bool
abstract_dom_variable_replacer::non_replaceable_node(
  const boost::property_tree::ptree& node)
{ // this is for YAML
    return false;
}
std::string
abstract_dom_variable_replacer::replace_value(
  const boost::property_tree::ptree& node,
  property_replacer replacer,
  bool fail_fast,
  const std::string& value)
{
    std::string sb = "";
    sb = sb + value;
    std::string replacer_prefix = "$" + replacer.get_prefix() + "{";
    int end_index = -1;
    int start_index = (int)sb.find(replacer_prefix);
    while (start_index > -1) {
        end_index = (int)sb.find('}', start_index);
        if (end_index == -1) {
            std::cout << "WARNING: "
                      << "Bad variable syntax. Could not find a closing curly bracket '}' for prefix " +
                           replacer_prefix + " on node: " + "node_name"
                      << std::endl; // TODO
            break;
        }
        std::string variable =
          sb.substr(start_index + replacer_prefix.length(), end_index - replacer_prefix.length());
        std::string variable_replacement = replacer.get_replacement(variable);
        if (!variable_replacement.empty()) {
            sb.replace(start_index, end_index + 1, variable_replacement);
            end_index = start_index + (int)variable_replacement.length();
        } else {
            handle_missing_variable(sb.substr(start_index, end_index + 1),
                                    "node_name",
                                    fail_fast); // TODO "node_name" is misssing
        }
        start_index = (int)sb.find(replacer_prefix, end_index);
    }
    return sb;
}
void
abstract_dom_variable_replacer::replace_variable_in_node_value(
  boost::property_tree::ptree* node,
  property_replacer replacer,
  bool fail_fast)
{
    if (non_replaceable_node(*node)) {
        return;
    }
    std::string value = node->data();
    if (!value.empty()) {
        std::string replaced_value =
          replace_value(*node, std::move(replacer), fail_fast, value);
        node->put_value(replaced_value);
    }
}

xml_dom_variable_replacer::xml_dom_variable_replacer() = default;
void
xml_dom_variable_replacer::replace_variables(
  boost::property_tree::ptree* node,
  property_replacer replacer,
  bool fail_fast)
{
    replace_variable_in_node_value(node, std::move(replacer), fail_fast);
}

}
}
}
}

