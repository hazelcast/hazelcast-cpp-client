//
// Created by Hakan Aktaş on 7.09.2022.
//
#include <hazelcast/client/client_config.h>
#include <boost/property_tree/ptree.hpp>
#include "config_replacer.h"
#include "dom_variable_replacer.h"
#ifndef HAZELCAST_CPP_CLIENT_ABSTRACT_DOM_CONFIG_REPLACER_H
#define HAZELCAST_CPP_CLIENT_ABSTRACT_DOM_CONFIG_REPLACER_H

class abstract_dom_variable_replacer : public dom_variable_replacer{

private:
    bool non_replaceable_node(boost::property_tree::ptree node);
    static void handle_missing_variable(std::string variable, std::string node_name, bool fail_fast);
protected:
    static std::string replace_value(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast, std::string value, std::string node_name);
public:
    void replace_variable_in_node_value(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast, std::string node_name);
};

#endif // HAZELCAST_CPP_CLIENT_ABSTRACT_DOM_CONFIG_REPLACER_H
