//
// Created by Hakan Aktaş on 7.09.2022.
//
#include "config_replacer.h"
#include <boost/property_tree/ptree.hpp>
#ifndef HAZELCAST_CPP_CLIENT_DOM_VARIABLE_REPLACER_H
#define HAZELCAST_CPP_CLIENT_DOM_VARIABLE_REPLACER_H
class dom_variable_replacer{
public:
    virtual void replace_variables(boost::property_tree::ptree node,config_replacer  replacer, bool fail_fast);
};
#endif // HAZELCAST_CPP_CLIENT_DOM_VARIABLE_REPLACER_H
