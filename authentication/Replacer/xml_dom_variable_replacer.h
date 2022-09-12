//
// Created by Hakan Aktaş on 7.09.2022.
//

#ifndef HAZELCAST_CPP_CLIENT_XML_DOM_VARIABLE_REPLACER_H
#define HAZELCAST_CPP_CLIENT_XML_DOM_VARIABLE_REPLACER_H
#include "abstract_dom_variable_replacer.h"
class xml_dom_variable_replacer : public abstract_dom_variable_replacer{
public:
    xml_dom_variable_replacer();
    void replace_variables(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast) override;

};

#endif // HAZELCAST_CPP_CLIENT_XML_DOM_VARIABLE_REPLACER_H
