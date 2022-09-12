//
// Created by Hakan Aktaş on 1.09.2022.
//
#include "abstract_dom_config_processor.h"

explicit abstract_dom_config_processor::abstract_dom_config_processor(bool dom_level_3){
    this->dom_level_3 = dom_level_3;
    this->strict = true;
}
abstract_dom_config_processor::abstract_dom_config_processor(bool dom_level_3, bool strict){
    this->dom_level_3 = dom_level_3;
    this->strict = strict;
}

bool abstract_dom_config_processor::matches(std::string config1, std::string config2){
    return config1 == config2;
}
std::string abstract_dom_config_processor::get_attribute(boost::property_tree::ptree node, std::string attribute){
    return node.get_child("<xmlattr>." + attribute).data();
}
bool abstract_dom_config_processor::get_bool_value(std::string value){
    boost::algorithm::to_lower(value);
    if(value == "true"){
        return true;
    }
    else if(value == "false"){
        return false;
    }
}
int abstract_dom_config_processor::get_integer_value(std::string parameter_name, std::string value){
    try{
        return std::stoi(value);
    }catch (const std::exception& e){
        throw new hazelcast::client::exception::invalid_configuration("Invalid integer value for parameter "+parameter_name+": "+value);
    }
}
int abstract_dom_config_processor::get_integer_value(std::string parameter_name, std::string value, int default_value){
    if(value.empty()){
        return default_value;
    }
    return get_integer_value(parameter_name, value);
}
long abstract_dom_config_processor::get_long_value(std::string parameter_name, std::string value){
    try{
        return std::stol(value);
    }catch (const std::exception& e){
        throw new hazelcast::client::exception::invalid_configuration("Invalid long value for parameter "+parameter_name+": "+value);
    }
}
long abstract_dom_config_processor::get_long_value(std::string parameter_name, std::string value, long default_value){
    if(value.empty()){
        return default_value;
    }
    return get_long_value(parameter_name, value);
}
double abstract_dom_config_processor::get_double_value(std::string parameter_name, std::string value){
    try{
        return std::stod(value);
    }catch (const std::exception& e){
        throw new hazelcast::client::exception::invalid_configuration("Invalid double value for parameter "+parameter_name+": "+value);
    }
}
double abstract_dom_config_processor::get_double_value(std::string parameter_name, std::string value, double default_value){
    if(value.empty()){
        return default_value;
    }
    return get_double_value(parameter_name, value);
}




hazelcast::client::serialization_config abstract_dom_config_processor::parse_serialization(boost::property_tree::ptree node){
    auto serialization_config = new hazelcast::client::serialization_config();
    for (auto& child : node) {
        std::string name = child.first;
        if (matches("portable-version", name)) {
            std::string value = child.second.data();
            serialization_config->set_portable_version(get_integer_value(child.first,value));
        } else if (matches("check-class-def-errors",  name)) {
            std::string value = child.second.data();
            //serializationConfig.setCheckClassDefErrors(getBooleanValue(value));
        } else if (matches("use-native-byte-order" , name)) {
            serialization_config->set_byte_order(boost::endian::order::native);
        } else if (matches("byte-order" , name)) {
            std::string value = child.second.data();
            if (matches("BIG_ENDIAN", value)) {
                serialization_config->set_byte_order(boost::endian::order::big);
            } else if (matches("LITTLE_ENDIAN", value)) {
                serialization_config->set_byte_order(
                  boost::endian::order::little);
            }
        } /*else if (matches("enable-compression", name)) {
            serializationConfig.setEnableCompression(getBooleanValue(getTextContent(child)));
        } else if (matches("enable-shared-object", name)) {
            serializationConfig.setEnableSharedObject(getBooleanValue(getTextContent(child)));
        } else if (matches("allow-unsafe", name)) {
            serializationConfig.setAllowUnsafe(getBooleanValue(getTextContent(child)));
        } else if (matches("allow-override-default-serializers", name)) {
            serializationConfig.setAllowOverrideDefaultSerializers(getBooleanValue(getTextContent(child)));
        } else if (matches("data-serializable-factories", name)) {
            fillDataSerializableFactories(child, serializationConfig);
        } else if (matches("portable-factories", name)) {
            fillPortableFactories(child, serializationConfig);
        } else if (matches("serializers", name)) {
            fillSerializers(child, serializationConfig);
        } else if (matches("java-serialization-filter", name)) {
            fillJavaSerializationFilter(child, serializationConfig);
        } else if (matches("compact-serialization", name)) {
            handleCompactSerialization(child, serializationConfig);
        }*/
    }
    return *serialization_config;
}
void abstract_dom_config_processor::fill_properties(boost::property_tree::ptree node, hazelcast::client::client_properties properties){
   fill_properties(node, std::move(properties), dom_level_3);
}
void abstract_dom_config_processor::fill_properties(const boost::property_tree::ptree& node, hazelcast::client::client_properties properties, bool dom_level_3){

}
void abstract_dom_config_processor::fill_properties(const boost::property_tree::ptree& node, std::unordered_map<std::string , std::string> properties, bool dom_level_3){

}

boost::property_tree::ptree abstract_dom_config_processor::pair_to_node(std::string node_name, boost::property_tree::ptree node_content){
    boost::property_tree::ptree temp;
    temp.add_child(node_name, node_content);
    return temp.get_child(node_name);
}