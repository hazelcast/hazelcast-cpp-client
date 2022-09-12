//
// Created by Hakan Aktaş on 31.08.2022.
//

#include "xml_client_config_builder.h"
hazelcast::logger logger = hazelcast::logger();
xml_client_config_builder::xml_client_config_builder(std::string resource){
    std::ifstream stream(resource, std::ifstream::in);
    this->in = &stream;
}
xml_client_config_builder::xml_client_config_builder(std::FILE file){
    if(file == nullptr){
        throw std::invalid_argument("File is null");
    }


}
xml_client_config_builder::xml_client_config_builder(std::ifstream* in){
    this->in = in;
}
xml_client_config_builder::xml_client_config_builder(){
    this(new xml_client_config_locator);
}
xml_client_config_builder::xml_client_config_builder(xml_client_config_locator locator){
    this->in = locator.get_in();
}


boost::property_tree::ptree xml_client_config_builder::parse(std::ifstream* input_stream) override{
    boost::property_tree::ptree tree;
    try{
        boost::property_tree::read_xml(input_stream,tree);
        input_stream.close();
        return tree;
    }catch (){
        input_stream.close();
    }

}
xml_client_config_builder xml_client_config_builder::set_properties(hazelcast::client::client_properties properties){
    set_properties_internal(properties);
    return this;
}
hazelcast::client::client_config xml_client_config_builder::build(){
    auto client_config = new hazelcast::client::client_config();
    parse_and_build_config(client_config);
    in.close();
    return client_config;
}

void xml_client_config_builder::parse_and_build_config(hazelcast::client::client_config client_config){
    auto root = parse(std::move(in));
    check_root_element(root);
    try {
        root.data();
    }catch (std::exception e){
        dom_level_3 = false;
    }
    process(root);
    if(should_validate_the_schema()){
        schema_validation(root);
    }
    client_dom_config_processor x(false, dom_level_3, client_config);
    x.build_config(root);

}
void xml_client_config_builder::check_root_element(boost::property_tree::ptree root){
    std::string root_node_name = root.data();

}




