//
// Created by Hakan Aktaş on 31.08.2022.
//

#include "abstract_xml_config_helper.h"
#include <regex>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>





void abstract_xml_config_helper::schema_validation(boost::property_tree::ptree doc){
    xercesc::XMLPlatformUtils::Initialize();
    std::string LINE_SEPARATOR = "\n";
    std::string schema_location = doc.get_child("hazelcast.<xmlattr>.xsi:schemaLocation").data();
    std::regex e("^ +| +$| (?= )");
    schema_location = std::regex_replace(schema_location,e,"");
    std::vector<std::string> xsd_locations;
    boost::regex reg("(?<!\\G\\S+)\\s");
    boost::algorithm::split_regex(xsd_locations, schema_location,reg);

    for(std::string xsd_location : xsd_locations){
        if(xsd_location.empty()){
            continue ;
        }
        std::vector<std::string> temp;
        boost::regex re('[' + LINE_SEPARATOR + " ]+");
        boost::algorithm::split_regex(temp,xsd_location,re);
        std::string name_space = *temp.begin();
        std::string uri = *(temp.begin() + 1);
        if(name_space == xmlns && !boost::algorithm::ends_with(uri,hazelcast_schema_location)){
            if(logger.enabled(hazelcast::logger::level::warning)){
                logger.log(hazelcast::logger::level::warning,"Name of the hazelcast schema location[" + uri + "] is incorrect, using default");
            }
        }
        if(name_space == xmlns){
            xercesc::XercesDOMParser parser;
            parser.setValidationScheme(xercesc::XercesDOMParser::Val_Always);
            parser.setDoNamespaces(true);
            parser.setDoSchema(true);
            parser.setValidationSchemaFullChecking(true);
            parser.setValidationConstraintFatal(true);
            boost::trim(name_space);
            parser.setExternalSchemaLocation((name_space + " " + hazelcast_schema_location).c_str());
            try{
                parser.parse("/Users/hakanaktas/Hazelcast/Reference Manual/hazelcast-cpp-client/examples/distributed-map/basic/hazelcast-token-credentials.xml");
                if(parser.getErrorCount() > 0){
                    throw hazelcast::client::exception::invalid_configuration()
                }
            } catch (std::exception ){

            }


        }
    }
    xercesc::XMLPlatformUtils::Terminate();
}



std::string abstract_xml_config_helper::get_release_version(){
    return hazelcast::client::version().to_string();
}

