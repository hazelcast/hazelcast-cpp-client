//
// Created by Hakan Aktaş on 4.09.2022.
//
#include <hazelcast/client/hazelcast_client.h>
#include <boost/algorithm/string.hpp>
#ifndef HAZELCAST_CPP_CLIENT_DECLARATIVE_CONFIG_UTIL_H
#define HAZELCAST_CPP_CLIENT_DECLARATIVE_CONFIG_UTIL_H
class declarative_config_util {
private:
    declarative_config_util()= default;
public:
    static std::string SYSPROP_MEMBER_CONFIG;
    static std::string SYSPROP_CLIENT_CONFIG ;
    static std::string SYSPROP_CLIENT_FAILOVER_CONFIG;
    static std::vector<std::string> XML_ACCEPTED_SUFFIXES;
    static std::string XML_ACCEPTED_SUFFIXES_STRING ;
    static std::vector<std::string> YAML_ACCEPTED_SUFFIXES;
    static std::string YAML_ACCEPTED_SUFFIXES_STRING;
    static std::vector<std::string> ALL_ACCEPTED_SUFFIXES;
    static std::string ALL_ACCEPTED_SUFFIXES_STRING;
    static void validate_suffix_in_system_property(std::string property_key);
    static void throw_unaccepted_suffix_in_system_property(std::string property_key, std::string config_resource, std::vector<std::string> accepted_suffixes);
    static bool is_accepted_suffix_configured(std::string config_file, std::vector<std::string> accepted_suffixes);

};


#endif // HAZELCAST_CPP_CLIENT_DECLARATIVE_CONFIG_UTIL_H
