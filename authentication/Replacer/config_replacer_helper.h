//
// Created by Hakan Aktaş on 7.09.2022.
//
#include <boost/property_tree/ptree.hpp>
#include "config_replacer.h"
#include "dom_variable_replacer.h"
#include "../DomConfigProcessor/abstract_dom_config_processor.h"
#ifndef HAZELCAST_CPP_CLIENT_CONFIG_REPLACER_HELPER_H
#define HAZELCAST_CPP_CLIENT_CONFIG_REPLACER_HELPER_H

class config_replacer_helper{
private:
    config_replacer_helper();
    static void traverse_children_and_replace_variables(boost::property_tree::ptree root, config_replacer replacer, bool fail_fast,
                                                        dom_variable_replacer variable_replacer);
    static boost::property_tree::ptree pair_to_node(std::string node_name, boost::property_tree::ptree node_content);

public:
    static void traverse_children_and_replace_variables(boost::property_tree::ptree root, std::vector<config_replacer> replacers, bool fail_fast,
                                                                                dom_variable_replacer variable_replacer);
};


#endif // HAZELCAST_CPP_CLIENT_CONFIG_REPLACER_HELPER_H
