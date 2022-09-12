//
// Created by Hakan Aktaş on 7.09.2022.
//

#include "abstract_dom_variable_replacer.h"
void abstract_dom_variable_replacer::handle_missing_variable(std::string variable, std::string node_name, bool fail_fast){
        std::string message = "Could not find a replacement for " + variable + " on node " + node_name;
        if(fail_fast){
            throw hazelcast::client::exception::invalid_configuration(message);
        }
        std::cout << "WARNING: " << message << std::endl;
}
bool abstract_dom_variable_replacer::non_replaceable_node(boost::property_tree::ptree node){//this is for YAML
    return false;
}
std::string abstract_dom_variable_replacer::replace_value(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast, std::string value, std::string node_name) {
    std::string sb = "";
    sb = sb + value;
    std::string replacer_prefix = "$" + replacer.get_prefix() + "{";
    int end_index = -1;
    int start_index = sb.find(replacer_prefix);
    while(start_index > -1){
        end_index = sb.find("}",start_index);
        if (end_index == -1) {
            std::cout << "WARNING: " << "Bad variable syntax. Could not find a closing curly bracket '}' for prefix " + replacer_prefix
                           + " on node: " + node_name << std::endl;
            break;
        }
        std::string variable = sb.substr(start_index + replacer_prefix.length(), end_index);
        std::string variable_replacement = replacer.get_replacement(variable);
        if(variable_replacement != ""){
            sb.replace(start_index,end_index+1,variable_replacement);
            end_index = start_index + variable_replacement.length();
        }
        else{
            handle_missing_variable(sb.substr(start_index, end_index + 1 ),node_name,fail_fast);
        }
        start_index = sb.find(replacer_prefix, end_index);
    }
    return sb;
}
void abstract_dom_variable_replacer::replace_variable_in_node_value(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast , std::string node_name){
    if(non_replaceable_node(node)){
        return ;
    }
    std::string value = node.data();
    if(value != ""){
        std::string replaced_value = replace_value(node, replacer, fail_fast, value, node_name);
        node.put_value(replaced_value);
    }

}