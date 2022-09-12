//
// Created by Hakan Aktaş on 31.08.2022.
//
#include "abstract_config_builder.h"
#ifndef HAZELCAST_CPP_CLIENT_ABSTRACT_XML_CONFIG_HELPER_H
#define HAZELCAST_CPP_CLIENT_ABSTRACT_XML_CONFIG_HELPER_H

class abstract_xml_config_helper: public abstract_config_builder
{
    std::string xmlns =  "http://www.hazelcast.com/schema/" + get_namespace_type();
    const std::string hazelcast_schema_location = "hazelcast-client-config-" + get_release_version() + ".xsd";

private:
    std::string xml_ref_to_java_name(const std::string name);
    static hazelcast::logger logger;
protected:

    bool dom_level_3 = true;
    void schema_validation(boost::property_tree::ptree doc);
    boost::asio::ip::tcp::iostream  load_schema_file(std::string schema_location);
    std::string xml_to_java_name(const std::string name);




public:
    std::string get_release_version();
    std::string get_namespace_type();


};

#endif // HAZELCAST_CPP_CLIENT_ABSTRACT_XML_CONFIG_HELPER_H
