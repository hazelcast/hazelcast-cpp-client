//
// Created by Hakan Aktaş on 31.08.2022.
//
#include "abstract_xml_config_builder.h"
#include "hazelcast/client/hazelcast_client.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "/Users/hakanaktas/Hazelcast/Reference Manual/hazelcast-cpp-client/examples/authentication/DomConfigProcessor/client_dom_config_processor.h"
#include "../Locator/xml_client_config_locator.h"

#ifndef HAZELCAST_CPP_CLIENT_XML_CLIENT_CONFIG_BUILDER_H
#define HAZELCAST_CPP_CLIENT_XML_CLIENT_CONFIG_BUILDER_H

class xml_client_config_builder: public abstract_xml_config_builder
{
private:
    static hazelcast::logger logger;
    std::ifstream* in;
    void parse_and_build_config(hazelcast::client::client_config client_config);
    void check_root_element(boost::property_tree::ptree root);


protected:
    boost::property_tree::ptree parse(std::ifstream input_stream);
public:
    xml_client_config_builder(std::string resource);
    xml_client_config_builder(std::ifstream* in);
    xml_client_config_builder();
    xml_client_config_builder(xml_client_config_locator locator);
    hazelcast::client::client_config build();
    xml_client_config_builder set_properties(hazelcast::client::client_properties properties);




};

#endif // HAZELCAST_CPP_CLIENT_XML_CLIENT_CONFIG_BUILDER_H
