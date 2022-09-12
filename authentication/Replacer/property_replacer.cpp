//
// Created by Hakan Aktaş on 12.09.2022.
//
#include "property_replacer.h"

property_replacer::property_replacer()= default;
void property_replacer::init(hazelcast::client::client_properties properties){
    this->properties = properties;
}
std::string property_replacer::get_prefix() {
    return "";
}
std::string property_replacer::get_replacement(std::string variable){
    return properties.getProperty(variable);
}