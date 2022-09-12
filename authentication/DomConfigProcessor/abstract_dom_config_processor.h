//
// Created by Hakan Aktaş on 1.09.2022.
//
#include <hazelcast/client/hazelcast_client.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#ifndef HAZELCAST_CPP_CLIENT_ABSTRACT_DOM_CONFIG_PROCESSOR_H
#define HAZELCAST_CPP_CLIENT_ABSTRACT_DOM_CONFIG_PROCESSOR_H

class abstract_dom_config_processor
{
protected:
    bool strict;
    bool dom_level_3;
    bool matches(std::string config1, std::string config2);
    hazelcast::client::serialization_config parse_serialization(boost::property_tree::ptree node);
    void fill_properties(boost::property_tree::ptree node, hazelcast::client::client_properties properties);
    static std::string get_attribute(boost::property_tree::ptree node, std::string attribute);
    static bool get_bool_value(std::string value);
    static int get_integer_value(std::string parameter_name, std::string value);
    static int get_integer_value(std::string parameter_name, std::string value, int default_value);
    static long get_long_value(std::string parameter_name, std::string value);
    static long get_long_value(std::string parameter_name, std::string value, long default_value);
    static double get_double_value(std::string parameter_name, std::string value);
    static double get_double_value(std::string parameter_name, std::string value, double default_value);
public:
    static boost::property_tree::ptree pair_to_node(std::string node_name, boost::property_tree::ptree node_content);
    std::unordered_set<std::string> * occurrence_set = new std::unordered_set<std::string>();
    static void fill_properties(const boost::property_tree::ptree& node, hazelcast::client::client_properties properties, bool dom_level_3);
    static void fill_properties(const boost::property_tree::ptree& node, std::unordered_map<std::string , std::string> properties, bool dom_level_3);
    explicit abstract_dom_config_processor(bool dom_level_3);
    abstract_dom_config_processor(bool dom_level_3, bool strict);
};

#endif // HAZELCAST_CPP_CLIENT_ABSTRACT_DOM_CONFIG_PROCESSOR_H
