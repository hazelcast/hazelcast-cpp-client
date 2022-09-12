//
// Created by Hakan Aktaş on 7.09.2022.
//

#include "config_replacer_helper.h"

config_replacer_helper::config_replacer_helper()= default;

void config_replacer_helper::traverse_children_and_replace_variables(boost::property_tree::ptree root, config_replacer replacer, bool fail_fast,
                                        dom_variable_replacer variable_replacer) {
    try{
        auto attributes = root.get_child("<xmlattr>");
        for(auto& attribute : attributes){
            auto attr = pair_to_node(attribute.first,attribute.second);
            variable_replacer.replace_variables(attr,replacer,fail_fast );
        }
        if(root.data() != ""){
            variable_replacer.replace_variables(root,replacer,fail_fast );
        }
        for(auto& pair : root){
            auto child = pair_to_node(pair.first,pair.second);
            traverse_children_and_replace_variables(child, replacer,fail_fast, variable_replacer);
        }
        }
        catch (const boost::exception& e){

        }
}
void config_replacer_helper::traverse_children_and_replace_variables(boost::property_tree::ptree root, std::vector<config_replacer> replacers, bool fail_fast,
                                                                dom_variable_replacer variable_replacer) {
    for(config_replacer replacer : replacers){
        traverse_children_and_replace_variables(root, replacer, fail_fast, variable_replacer);
    }

}
boost::property_tree::ptree abstract_dom_config_processor::pair_to_node(std::string node_name, boost::property_tree::ptree node_content){
    boost::property_tree::ptree temp;
    temp.add_child(node_name, node_content);
    return temp.get_child(node_name);
}