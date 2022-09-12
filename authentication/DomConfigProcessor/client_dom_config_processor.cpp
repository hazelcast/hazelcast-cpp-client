//
// Created by Hakan Aktaş on 1.09.2022.
//

#include "client_dom_config_processor.h"
void client_dom_config_processor::build_config(const boost::property_tree::ptree& root_node){
    for(auto& pair : root_node){
        std::string node_name = pair.first;
        auto node = pair_to_node(pair.first, pair.second);
        if(occurrence_set->find(node_name) == occurrence_set->end()){
            handle_node(node, node_name);
        }
        else{
            throw hazelcast::client::exception::invalid_configuration("Duplicate '" + node_name + "' definition found in the configuration");
        }
        if(!client_config_sections::can_occur_multiple_times(node_name)){
            occurrence_set->insert(node_name);
        }
    }
}
void client_dom_config_processor::handle_node(const boost::property_tree::ptree& node, std::string &node_name){
    if(matches(node_name,client_config_sections::SECURITY.get_name())){
        handle_security(node);
    }
    else if(matches(node_name,client_config_sections::PROXY_FACTORIES.get_name())){
        handle_proxy_factories(node);
    }
    else if(matches(node_name,client_config_sections::PROPERTIES.get_name())){
        fill_properties(node, client_config->get_properties());
    }
    else if(matches(node_name,client_config_sections::SERIALIZATION.get_name())){
        handle_serialization(node);
    }
    else if(matches(node_name,client_config_sections::NATIVE_MEMORY.get_name())){//not supported

    }
    else if(matches(node_name,client_config_sections::LISTENERS.get_name())){
        handle_listeners(node);
    }
    else if(matches(node_name,client_config_sections::NETWORK.get_name())){
        handle_network(node);
    }
    else if(matches(node_name,client_config_sections::LOAD_BALANCER.get_name())){
        handle_load_balancer(node);
    }
    else if(matches(node_name,client_config_sections::NEAR_CACHE.get_name())){
        handle_near_cache(node);
    }
    else if(matches(node_name,client_config_sections::QUERY_CACHES.get_name())){//not supported

    }
    else if(matches(node_name,client_config_sections::INSTANCE_NAME.get_name())){
        client_config->set_instance_name(node.data());
    }
    else if(matches(node_name,client_config_sections::CONNECTION_STRATEGY.get_name())){
        handle_connection_strategy(node);
    }
    else if(matches(node_name,client_config_sections::USER_CODE_DEPLOYMENT.get_name())){//not supported

    }
    else if(matches(node_name,client_config_sections::FLAKE_ID_GENERATOR.get_name())){
        handle_flake_id_generator(node);
    }
    else if(matches(node_name,client_config_sections::RELIABLE_TOPIC.get_name())){
        handle_reliable_topic(node);
    }
    else if(matches(node_name,client_config_sections::LABELS.get_name())){
        handle_labels(node);
    }
    else if(matches(node_name,client_config_sections::BACKUP_ACK_TO_CLIENT.get_name())){
        handle_backup_ack_to_client(node);
    }
    else if(matches(node_name,client_config_sections::CLUSTER_NAME.get_name())){
        client_config->set_cluster_name(node.data());
    }
    else if(matches(node_name,client_config_sections::METRICS.get_name())){//?????

    }
    else if(matches(node_name,client_config_sections::INSTANCE_TRACKING.get_name())){//?????

    }
}

void client_dom_config_processor::handle_security(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("username-password", node_name)) {
            client_config->set_credentials(
              std::make_shared<
                hazelcast::client::security::username_password_credentials>(
                get_attribute(child,"username"), get_attribute(child, "password")));
        } else if (matches("token", node_name)) {
            client_config->set_credentials(std::make_shared<hazelcast::client::security::token_credentials>(pair.second.data()));
        } else if (matches("credentials-factory", node_name)) {//not supported

        } else if (matches("kerberos", node_name)) {//not supported

        } else if (matches("realms", node_name)) {
            handle_realms(child);
        }
    }

}
void client_dom_config_processor::handle_serialization(const boost::property_tree::ptree& node){
    hazelcast::client::serialization_config serialization_config = parse_serialization(node);
    client_config->set_serialization_config(serialization_config);
}
void client_dom_config_processor::handle_network(const boost::property_tree::ptree& node){
    auto client_network_config = new hazelcast::client::config::client_network_config();
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("cluster-members", node_name)) {
            handle_cluster_members(child, client_network_config);
        } else if (matches("smart-routing",node_name)) {
            client_network_config->set_smart_routing(get_bool_value(pair.second.data()));
        } else if (matches("redo-operation", node_name)) {
            client_config->set_redo_operation(get_bool_value(pair.second.data()));
        } else if (matches("connection-timeout", node_name)){
            client_network_config->set_connection_timeout(std::chrono::milliseconds(get_integer_value(node_name, pair.second.data())));
        } else if (matches("socket-options", node_name)) {
            handle_socket_options(child, client_network_config);
        } else if (matches("socket-interceptor", node_name)) {//not supported

        } else if (matches("ssl" , node_name)) {
            handle_ssl_config(child, client_network_config);
        } //else if (AliasedDiscoveryConfigUtils.supports(node_name)) {//not supported
            // handleAliasedDiscoveryStrategy(child, clientNetworkConfig, nodeName);}
        else if (matches("discovery-strategies", node_name)) {//not supported

        } else if (matches("auto-detection", node_name)) {//not supported

        } else if (matches("outbound-ports", node_name)) {//not supported

        } else if (matches("icmp-ping", node_name)) {//not supported

        } else if (matches("hazelcast-cloud" , node_name)) {
            handle_hazelcast_cloud(child, client_network_config);
        }
    }
    client_config->set_network_config(*client_network_config);
}

void client_dom_config_processor::handle_cluster_members(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){
    for(auto& pair : node){
        if(matches("address" , pair.first)){
            int port;//port is not included in the example usage in hazelcast-client-full-example.xml
            client_network_config->add_address(hazelcast::client::address(pair.second.data(),port));
        }
    }
}

void client_dom_config_processor::handle_socket_options(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("tcp-no-delay", node_name)) {
            client_network_config->get_socket_options().set_tcp_no_delay(get_bool_value(pair.second.data()));
        } else if (matches("keep-alive", node_name)) {
            client_network_config->get_socket_options().set_keep_alive(get_bool_value(pair.second.data()));
        } else if (matches("reuse-address" , node_name)) {
            client_network_config->get_socket_options().set_reuse_address(get_bool_value(pair.second.data()));
        } else if (matches("linger-seconds" , node_name)) {
            client_network_config->get_socket_options().set_linger_seconds(get_integer_value(pair.first, pair.second.data()));
        } else if (matches("buffer-size" ,node_name)) {
            client_network_config->get_socket_options().set_buffer_size_in_bytes(get_integer_value(pair.first, pair.second.data()));
        }
    }
}
void client_dom_config_processor::handle_ssl_config(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){

}
void client_dom_config_processor::handle_hazelcast_cloud(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){
    bool enabled = false;
    try{
        std::string enable = get_attribute(node, "enabled");
        boost::algorithm::trim(enable);
        enabled = get_bool_value(enable);
    }catch (const boost::exception& e){

    }
    client_network_config->get_cloud_config().enabled = enabled;
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if(matches("discovery-token" , pair.first)){
            client_network_config->get_cloud_config().discovery_token = pair.second.data();
        }
    }
}
void client_dom_config_processor::client_dom_config_processor::handle_flake_id_generator(const boost::property_tree::ptree& node){
    handle_flake_id_generator_node(node);
}

void client_dom_config_processor::client_dom_config_processor::handle_flake_id_generator_node(const boost::property_tree::ptree& node){
    std::string name = get_attribute(node, "name");
    auto config = new hazelcast::client::config::client_flake_id_generator_config(name);
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        boost::algorithm::to_lower(node_name);
        if(matches("prefetch-count" , node_name)){
            config->set_prefetch_count(get_integer_value(pair.first, pair.second.data()));
        }
        else if(matches("prefetch-validity-millis" ,node_name )){
            config->set_prefetch_validity_duration(std::chrono::milliseconds(get_long_value(node_name, pair.second.data())));
        }

    }
    client_config->add_flake_id_generator_config(*config);
}

void client_dom_config_processor::handle_reliable_topic(const boost::property_tree::ptree& node){
    handle_reliable_topic_node(node);
}
void client_dom_config_processor::handle_reliable_topic_node(const boost::property_tree::ptree& node){
    std::string name = get_attribute(node, "name");
    auto config = new hazelcast::client::config::reliable_topic_config(name);
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if(matches("topic-overload-policy" , node_name)){//doesnt exist

        }
        else if(matches("read-batch-size" , node_name)){
            config->set_read_batch_size(get_integer_value(pair.first, pair.second.data()));
        }
    }
    client_config->add_reliable_topic_config(*config);
}
void client_dom_config_processor::handle_near_cache(const boost::property_tree::ptree& node){
    handle_near_cache_node(node);
}
void client_dom_config_processor::handle_near_cache_node(const boost::property_tree::ptree& node){
    hazelcast::client::config::near_cache_config * near_cache_config;
    try{
        std::string name = get_attribute(node,"name");
        near_cache_config = new hazelcast::client::config::near_cache_config(name);
    } catch(const boost::exception& e){
        near_cache_config = new hazelcast::client::config::near_cache_config("default");
    }
    for (auto& pair: node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("time-to-live-seconds" , node_name)) {
            near_cache_config->set_time_to_live_seconds(get_integer_value(pair.first, pair.second.data()));
        } else if (matches("max-idle-seconds" , node_name)) {
            near_cache_config->set_max_idle_seconds(get_integer_value(pair.first, pair.second.data()));
        } else if (matches("in-memory-format" , node_name)) {//NO NATIVE ??
            if(pair.second.data() == "BINARY"){
                near_cache_config->set_in_memory_format(hazelcast::client::config::in_memory_format::BINARY);
            }
            else if(pair.second.data() == "OBJECT"){
                near_cache_config->set_in_memory_format(hazelcast::client::config::in_memory_format::OBJECT);
            }
        } else if (matches("serialize-keys", node_name)) {//??
            //serializeKeys = Boolean.parseBoolean(getTextContent(child));
            //nearCacheConfig.setSerializeKeys(serializeKeys);
        } else if (matches("invalidate-on-change", node_name)) {
            near_cache_config->set_invalidate_on_change(get_bool_value(pair.second.data()));
        } else if (matches("local-update-policy", node_name)){
            if(pair.second.data() == "CACHE"){
                near_cache_config->set_local_update_policy(hazelcast::client::config::near_cache_config::CACHE);
            }
            else if (pair.second.data() == "INVALIDATE"){
                near_cache_config->set_local_update_policy(hazelcast::client::config::near_cache_config::INVALIDATE);
            }
        } else if (matches("eviction" , node_name)) {
            near_cache_config->set_eviction_config(get_eviction_config(pair.second));
        } else if (matches("preloader", node_name)) {//??
           // nearCacheConfig.setPreloaderConfig(getNearCachePreloaderConfig(child));
        }
    }
    /*if (serializeKeys != null && !serializeKeys && nearCacheConfig.getInMemoryFormat() == InMemoryFormat.NATIVE) {
        std::cout << "WARNING: " << "The Near Cache doesn't support keys by-reference with NATIVE in-memory-format. This setting will have no effect!" << std::endl;
    }*/
    client_config->add_near_cache_config(*near_cache_config);
}
hazelcast::client::config::eviction_config client_dom_config_processor::get_eviction_config(const boost::property_tree::ptree& node){
    auto eviction_config = hazelcast::client::config::eviction_config();
    try{
        auto attr = get_attribute(node, "size");
        eviction_config.set_size(get_integer_value("size", attr));
    }catch (const boost::exception& e){

    }
    try{
        auto attr = get_attribute(node, "max-size-policy");
        if(attr == "ENTRY_COUNT"){// other policies are not implemented
            eviction_config.set_maximum_size_policy(hazelcast::client::config::eviction_config::ENTRY_COUNT);
        }
    } catch (const boost::exception& e){

    }
    try{
        auto attr = get_attribute(node, "eviction-policy");
        if(attr == "LRU"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::LRU);
        }
        else if(attr == "LFU"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::LFU);
        }
        else if(attr == "NONE"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::NONE);
        }
        else if(attr == "RANDOM"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::RANDOM);
        }
    } catch (const boost::exception& e){

    }
    try{
        get_attribute(node, "comparator-class-name");//couldn't find this option
    } catch (const boost::exception& e){

    }
    return eviction_config;
}
void client_dom_config_processor::handle_connection_strategy(const boost::property_tree::ptree& node){
    auto strategy_config = hazelcast::client::config::client_connection_strategy_config();
    try{
        std::string attr_val = get_attribute(node, "async-start");
        boost::algorithm::trim(attr_val);
        strategy_config.set_async_start(get_bool_value(attr_val));
    }catch (const boost::exception& e){
        strategy_config.set_async_start(false);
    }
    try{
        std::string attr_val1 = get_attribute(node, "reconnect-mode");
        boost::algorithm::trim(attr_val1);
        if(attr_val1 == "ON"){
            strategy_config.set_reconnect_mode(hazelcast::client::config::client_connection_strategy_config::reconnect_mode::ON);
        } else if(attr_val1 == "OFF"){
            strategy_config.set_reconnect_mode(hazelcast::client::config::client_connection_strategy_config::reconnect_mode::OFF);
        } else if(attr_val1 == "ASYNC"){
            strategy_config.set_reconnect_mode(hazelcast::client::config::client_connection_strategy_config::reconnect_mode::ASYNC);
        }
    }catch (const boost::exception& e){

    }
    for(auto& pair : node ){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("connection-retry", node_name)) {
            handle_connection_retry(child,strategy_config);
        }
    }
    client_config->set_connection_strategy_config(strategy_config);
}
void client_dom_config_processor::handle_connection_retry(const boost::property_tree::ptree& node,hazelcast::client::config::client_connection_strategy_config strategy_config){
    auto connection_retry_config = new hazelcast::client::config::connection_retry_config();
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("initial-backoff-millis", node_name)) {
            connection_retry_config->set_initial_backoff_duration(std::chrono::milliseconds(get_integer_value(node_name, pair.second.data())));
        } else if (matches("max-backoff-millis", node_name)) {
            connection_retry_config->set_max_backoff_duration(std::chrono::milliseconds(get_integer_value(node_name, pair.second.data())));
        } else if (matches("multiplier", node_name)) {
            connection_retry_config->set_multiplier(get_double_value(node_name, pair.second.data()));
        } else if (matches("cluster-connect-timeout-millis", node_name)) {
            connection_retry_config->set_cluster_connect_timeout(std::chrono::milliseconds(get_long_value(node_name, pair.second.data())));
        } else if (matches("jitter", node_name)) {
            connection_retry_config->set_jitter(get_double_value(node_name, pair.second.data()));
        }
    }
    strategy_config.set_retry_config(*connection_retry_config);
}

void client_dom_config_processor::handle_labels(const boost::property_tree::ptree& node){
    for(auto& child : node){
        client_config->add_label(child.first);
    }
}
void client_dom_config_processor::handle_proxy_factories(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        handle_proxy_factories_node(child);
    }
}
void client_dom_config_processor::handle_proxy_factories_node(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if(matches(node_name , "proxy-factory")){//not sure if it is supported

        }
    }
}
void client_dom_config_processor::handle_listeners(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        if(matches(pair.first, "listener")){//not sure how to add listeners

        }
    }
}
void client_dom_config_processor::handle_backup_ack_to_client(const boost::property_tree::ptree& node){
    client_config->backup_acks_enabled(get_bool_value(node.data()));
}
void client_dom_config_processor::handle_realms(const boost::property_tree::ptree& node){
    for (auto& pair: node) {
        auto child = pair_to_node(pair.first, pair.second);
        if (matches("realm", pair.first)) {
            handle_realm(child);
        }
    }
}
void client_dom_config_processor::handle_realm(const boost::property_tree::ptree& node){
    try{
        auto realm_name = get_attribute(node, "name");
    }
    catch(const boost::exception& e){

    }
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("authentication", node_name)) {//authentication not supported

        }
    }
}
void client_dom_config_processor::handle_load_balancer(const boost::property_tree::ptree& node){//not sure how to configure load_balancer
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