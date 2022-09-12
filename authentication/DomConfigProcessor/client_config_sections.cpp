//
// Created by Hakan Aktaş on 5.09.2022.
//

#include "client_config_sections.h"
client_config_sections::client_config_sections(std::string name, bool multiple_occurrence){
    this->name = name;
    this->multiple_occurrence = multiple_occurrence;
}
static client_config_sections HAZELCAST_CLIENT("hazelcast-client", false);
static client_config_sections IMPORT("import", true);
static client_config_sections SECURITY("security", false);
static client_config_sections PROXY_FACTORIES("proxy-factories", false);
static client_config_sections PROPERTIES("properties", false);
static client_config_sections SERIALIZATION("serialization", false);
static client_config_sections NATIVE_MEMORY("native-memory", false);
static client_config_sections LISTENERS("listeners", false);
static client_config_sections NETWORK("network", false);
static client_config_sections LOAD_BALANCER("load-balancer", false);
static client_config_sections NEAR_CACHE("near-cache", true);
static client_config_sections QUERY_CACHES("query-caches", false);
static client_config_sections BACKUP_ACK_TO_CLIENT("backup-ack-to-client-enabled", false);
static client_config_sections INSTANCE_NAME("instance-name", false);
static client_config_sections CONNECTION_STRATEGY("connection-strategy", false);
static client_config_sections USER_CODE_DEPLOYMENT("user-code-deployment", false);
static client_config_sections FLAKE_ID_GENERATOR("flake-id-generator", true);
static client_config_sections RELIABLE_TOPIC("reliable-topic", true);
static client_config_sections LABELS("client-labels", false);
static client_config_sections CLUSTER_NAME("cluster-name", false);
static client_config_sections METRICS("metrics", false);
static client_config_sections INSTANCE_TRACKING("instance-tracking", false);

std::vector<client_config_sections> values = {HAZELCAST_CLIENT, IMPORT, SECURITY, PROXY_FACTORIES, PROPERTIES, SERIALIZATION, NATIVE_MEMORY,
                                               LISTENERS, NETWORK, LOAD_BALANCER, NEAR_CACHE, QUERY_CACHES, BACKUP_ACK_TO_CLIENT, INSTANCE_NAME,
                                                 CONNECTION_STRATEGY, USER_CODE_DEPLOYMENT, FLAKE_ID_GENERATOR, RELIABLE_TOPIC, LABELS, CLUSTER_NAME,
                                               METRICS, INSTANCE_TRACKING};

bool client_config_sections::can_occur_multiple_times(std::string name){
    for (client_config_sections section : values){
        if(name == section.get_name()){
            return section.multiple_occurrence;
        }
    }
    return true;
}

