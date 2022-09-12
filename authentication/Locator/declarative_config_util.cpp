//
// Created by Hakan Aktaş on 4.09.2022.
//
#include "declarative_config_util.h"

std::string SYSPROP_MEMBER_CONFIG = "hazelcast.config" ;
std::string SYSPROP_CLIENT_CONFIG = "hazelcast.client.config" ;
std::string SYSPROP_CLIENT_FAILOVER_CONFIG = "hazelcast.client.failover.config";
std::vector<std::string> XML_ACCEPTED_SUFFIXES = {"xml"};
std::string XML_ACCEPTED_SUFFIXES_STRING = "[xml]";
std::vector<std::string> YAML_ACCEPTED_SUFFIXES = {"yaml", "yml"};
std::string YAML_ACCEPTED_SUFFIXES_STRING = "[yaml, yml]";
std::vector<std::string> ALL_ACCEPTED_SUFFIXES = {"xml", "yaml", "yml"};
std::string ALL_ACCEPTED_SUFFIXES_STRING = "[xml, yaml, yml]";

void declarative_config_util::validate_suffix_in_system_property(std::string property_key){
    std::string config_system_property = System.getProperty(propertyKey);
    if (config_system_property.empty()) {
        return;
    }
    if (!is_accepted_suffix_configured(config_system_property, ALL_ACCEPTED_SUFFIXES)) {
        throw_unaccepted_suffix_in_system_property(property_key, config_system_property, ALL_ACCEPTED_SUFFIXES);
    }
}
void declarative_config_util::throw_unaccepted_suffix_in_system_property(std::string property_key, std::string config_resource, std::vector<std::string> accepted_suffixes){

    std::string message = "The suffix of the resource \'" + config_resource + "\' referenced in \'" + property_key + "\' is not in the list of accepted " + "suffixes: \'[" + boost::algorithm::join(accepted_suffixes, ", ") +"]\'";
    throw hazelcast::client::exception::hazelcast_(message);
}
bool declarative_config_util::is_accepted_suffix_configured(std::string config_file, std::vector<std::string> accepted_suffixes){
    std::string config_file_lower;
    if(config_file.empty()){
        config_file_lower = config_file;
    }
    else{
        config_file_lower = config_file;
        boost::algorithm::to_lower(config_file_lower);
    }
    int last_dot_index = config_file_lower.find_last_of(".");
    if(last_dot_index == -1){
        return false;
    }
    std::string config_file_suffix = config_file_lower.substr(last_dot_index + 1);
    return std::find(accepted_suffixes.begin(), accepted_suffixes.end(), config_file_suffix) != accepted_suffixes.end();
}