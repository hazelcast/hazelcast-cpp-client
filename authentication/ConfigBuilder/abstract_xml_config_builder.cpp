//
// Created by Hakan Aktaş on 31.08.2022.
//

#include <fstream>
#include <utility>
#include "abstract_xml_config_builder.h"
#include "/Users/hakanaktas/Hazelcast/Reference Manual/hazelcast-cpp-client/examples/authentication/DomConfigProcessor/client_config_sections.h"
#include "../Replacer/config_replacer_helper.h"
#include "../Replacer/xml_dom_variable_replacer.h"
#include "/Users/hakanaktas/Hazelcast/Reference Manual/hazelcast-cpp-client/examples/authentication/DomConfigProcessor/abstract_dom_config_processor.h"

std::string abstract_xml_config_builder::get_attribute(boost::property_tree::ptree node, std::string att_name){
    return node.get_child("<xmlattr>." + att_name).data();
}

void abstract_xml_config_builder::process(boost::property_tree::ptree root){
    replace_imports(root);
    replace_variables(root);
}
void abstract_xml_config_builder::replace_variables(boost::property_tree::ptree root){
    bool fail_fast = false;

    auto replacers = new std::vector<config_replacer>();
    property_replacer propertyReplacer;
    propertyReplacer.init(get_properties());
    replacers->push_back(propertyReplacer);
    boost::property_tree::ptree node;
    try{
        node = root.get_child("hazelcast-client.config-replacers");
        try{
            std::string fail_fast_attr = get_attribute(node, "fail-if-value-missing");
            if(fail_fast_attr == "true"){
                fail_fast = true;
            }
            for(auto& n : node){
                std::string value = n.first;
                if ("replacer" == value) {
                    replacers->push_back(create_replacer(n.second));
                }
            }
        }catch(const boost::exception& e){

        }
    }catch(const boost::exception& e){

    }
    config_replacer_helper::traverse_children_and_replace_variables(root, *replacers, fail_fast, *(new xml_dom_variable_replacer()));
}
void abstract_xml_config_builder::replace_imports(boost::property_tree::ptree root){
    replace_variables(root);

    //there is something to give error here


    for(auto& child : root){
        if(child.first == "import"){
            boost::property_tree::ptree temp;
            temp.add_child(child.first,child.second);
            std::string resource = get_attribute(temp.get_child(child.first),"resource");
            std::ifstream * stream;
            stream->open(resource);
            if(stream->fail()){
                throw hazelcast::client::exception::invalid_configuration("Failed to load resource: " + resource);
            }
            if(!currently_imported_files->emplace(resource).second){
                throw hazelcast::client::exception::invalid_configuration("Resource '" + resource + "' is already loaded! This can be due to"
                                                                          + " duplicate or cyclic imports.");
            }
            boost::property_tree::ptree imported_root = parse(stream).get_child("hazelcast-client");
            replace_imports(imported_root);
            for(auto& imported_node : imported_root){
                if(imported_node.first == "<xmlattr>"){
                    continue ;
                }
                root.put_child(imported_node.first, imported_node.second);
            }
            for(auto it = root.begin(); it != root.end(); it++){
                if(it->first == "import"){
                    root.erase(it);
                    break;
                }
            }
        }
    }

}

config_replacer abstract_xml_config_builder::create_replacer(boost::property_tree::ptree node){
    std::string replacer_class = get_attribute(node,"class-name");
    std::unordered_map<std::string, std::string> prop;
    hazelcast::client::client_properties properties(prop);
    for(auto& n : node){
        std::string value = n.first;
        if("properties" == value){
            fill_properties(n.second, properties);
        }
    }
    config_replacer replacer;
    replacer.init(properties);
    return replacer;
}
void abstract_xml_config_builder::fill_properties(boost::property_tree::ptree node, hazelcast::client::client_properties properties){
    abstract_dom_config_processor::fill_properties(node, std::move(properties), dom_level_3);
}
