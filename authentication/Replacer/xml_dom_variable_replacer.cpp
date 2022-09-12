//
// Created by Hakan Aktaş on 7.09.2022.
//

#include "xml_dom_variable_replacer.h"

xml_dom_variable_replacer::xml_dom_variable_replacer(){

}
void xml_dom_variable_replacer::replace_variables(boost::property_tree::ptree node, config_replacer replacer, bool fail_fast){
    replace_variable_in_node_value(node, replacer, fail_fast);
}