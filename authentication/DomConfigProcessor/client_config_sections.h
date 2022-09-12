//
// Created by Hakan Aktaş on 5.09.2022.
//
#include <hazelcast/client/hazelcast_client.h>
#ifndef HAZELCAST_CPP_CLIENT_CLIENT_CONFIG_SECTIONS_H
#define HAZELCAST_CPP_CLIENT_CLIENT_CONFIG_SECTIONS_H

class client_config_sections{
    bool multiple_occurrence;
private:
    std::string name;
    static std::vector<client_config_sections> values;
public:

    client_config_sections(std::string name, bool multiple_occurrence);
    static bool can_occur_multiple_times(std::string name);
    std::string get_name(){
        return name;
    }
    static client_config_sections HAZELCAST_CLIENT;
    static client_config_sections IMPORT;
    static client_config_sections SECURITY;
    static client_config_sections PROXY_FACTORIES;
    static client_config_sections PROPERTIES;
    static client_config_sections SERIALIZATION;
    static client_config_sections NATIVE_MEMORY;
    static client_config_sections LISTENERS;
    static client_config_sections NETWORK;
    static client_config_sections LOAD_BALANCER;
    static client_config_sections NEAR_CACHE;
    static client_config_sections QUERY_CACHES;
    static client_config_sections BACKUP_ACK_TO_CLIENT;
    static client_config_sections INSTANCE_NAME;
    static client_config_sections CONNECTION_STRATEGY;
    static client_config_sections USER_CODE_DEPLOYMENT;
    static client_config_sections FLAKE_ID_GENERATOR;
    static client_config_sections RELIABLE_TOPIC;
    static client_config_sections LABELS;
    static client_config_sections CLUSTER_NAME;
    static client_config_sections METRICS;
    static client_config_sections INSTANCE_TRACKING;

};



#endif // HAZELCAST_CPP_CLIENT_CLIENT_CONFIG_SECTIONS_H
