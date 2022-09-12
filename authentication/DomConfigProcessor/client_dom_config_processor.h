//
// Created by Hakan Aktaş on 1.09.2022.
//
#include "abstract_dom_config_processor.h"
#include "client_config_sections.h"

#ifndef HAZELCAST_CPP_CLIENT_CLIENT_DOM_CONFIG_PROCESSOR_H
#define HAZELCAST_CPP_CLIENT_CLIENT_DOM_CONFIG_PROCESSOR_H

class client_dom_config_processor: public abstract_dom_config_processor
{

private:

    void handle_node(const boost::property_tree::ptree& node, std::string &node_name);
    void handle_security(const boost::property_tree::ptree& node);
    void handle_network(const boost::property_tree::ptree& node);
    void handle_serialization(const boost::property_tree::ptree& node);
    void handle_socket_options(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config);
    void handle_ssl_config(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config);
    void handle_hazelcast_cloud(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config);
    hazelcast::client::config::eviction_config get_eviction_config(const boost::property_tree::ptree& node);
    void handle_connection_strategy(const boost::property_tree::ptree& node);
    void handle_labels(const boost::property_tree::ptree& node);
    void handle_proxy_factories(const boost::property_tree::ptree& node);
    void handle_backup_ack_to_client(const boost::property_tree::ptree& node);
    void handle_load_balancer(const boost::property_tree::ptree& node);
    void handle_connection_retry(const boost::property_tree::ptree& node,hazelcast::client::config::client_connection_strategy_config strategy_config);
protected:
    void handle_cluster_members(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config);
    void handle_flake_id_generator(const boost::property_tree::ptree& node);
    void handle_flake_id_generator_node(const boost::property_tree::ptree& node);
    void handle_reliable_topic(const boost::property_tree::ptree& node);
    void handle_reliable_topic_node(const boost::property_tree::ptree& node);
    void handle_near_cache(const boost::property_tree::ptree& node);
    void handle_near_cache_node(const boost::property_tree::ptree& node);
    void handle_proxy_factories_node(const boost::property_tree::ptree& node);
    void handle_listeners(const boost::property_tree::ptree& node);
    void handle_realms(const boost::property_tree::ptree& node);
    void handle_realm(const boost::property_tree::ptree& node);


public:
    hazelcast::client::client_config* client_config;
    client_dom_config_processor(bool dom_level_3, hazelcast::client::client_config* client_config) : abstract_dom_config_processor(dom_level_3){
        this->client_config = client_config;
    }
    client_dom_config_processor( bool dom_level_3, hazelcast::client::client_config* client_config, bool strict) : abstract_dom_config_processor(dom_level_3,strict){
        this->client_config = client_config;
    }

    void build_config(const boost::property_tree::ptree& root_node);





};
#endif // HAZELCAST_CPP_CLIENT_CLIENT_DOM_CONFIG_PROCESSOR_H
