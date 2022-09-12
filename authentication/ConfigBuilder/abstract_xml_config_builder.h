//
// Created by Hakan Aktaş on 31.08.2022.
//
#include "abstract_xml_config_helper.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/beast/http.hpp>
#include "../Replacer/property_replacer.h"
#include "../Replacer/config_replacer.h"
#ifndef HAZELCAST_CPP_CLIENT_ABSTRACT_XML_CONFIG_BUILDER_H
#define HAZELCAST_CPP_CLIENT_ABSTRACT_XML_CONFIG_BUILDER_H

class abstract_xml_config_builder: public abstract_xml_config_helper
{
    std::string get_attribute(boost::property_tree::ptree node, std::string att_name);

    void fill_properties(boost::property_tree::ptree node, hazelcast::client::client_properties properties);

private:
    hazelcast::client::client_properties properties ;
    std::unordered_set<std::string> * currently_imported_files = new std::unordered_set<std::string>();
    void replace_variables(boost::property_tree::ptree root);
    void replace_imports(boost::property_tree::ptree root);
    config_replacer create_replacer(boost::property_tree::ptree node);


protected:
    void process(boost::property_tree::ptree root);
    virtual boost::property_tree::ptree parse(std::ifstream* input_stream);

    void set_properties_internal(hazelcast::client::client_properties properties){
        this->properties = properties;
    }
public:
    hazelcast::client::client_properties get_properties(){
        return properties;
    }
    abstract_xml_config_builder();


};

#endif // HAZELCAST_CPP_CLIENT_ABSTRACT_XML_CONFIG_BUILDER_H
