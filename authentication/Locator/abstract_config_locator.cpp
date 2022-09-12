//
// Created by Hakan Aktaş on 3.09.2022.
//


#include "abstract_config_locator.h"
bool abstract_config_locator::locate_everywhere(){
    return locate_from_system_property_or_fail_on_unaccepted_suffix()
           || locate_in_work_directory()
           || locate_default();
}
bool abstract_config_locator::load_from_working_directory(std::string config_file_path){
    try{
        std::FILE* file;
        file = fopen(config_file_path.c_str(),"r");
        if(file == nullptr){
            std::cout << "FINEST: " << "Could not find " + config_file_path + " in the working directory." << std::endl;
            return false;
        }
        std::cout << "INFO: Loading " + config_file_path + " from the working directory." << std::endl;
        configuration_file = file;
        in->open(config_file_path,std::ios::in);
        if(in->fail()){
            throw hazelcast::client::exception::hazelcast_("Failed to open file: " + config_file_path);
        }
    }catch (const std::runtime_error& e){
        throw hazelcast::client::exception::hazelcast_(e.what());
    }
}

bool abstract_config_locator::load_from_working_directory(std::string config_file_prefix, std::vector<std::string> accepted_suffixes){
    if(accepted_suffixes.empty()){
        throw std::invalid_argument("Parameter acceptedSuffixes must not be empty");
    }
    for(const auto& suffix : accepted_suffixes){
        if(suffix.empty()){
            throw std::invalid_argument("Parameter acceptedSuffixes must not contain empty strings");
        }
        if(load_from_working_directory(config_file_prefix + "." + suffix)){
            return true;
        }
    }
    return false;
}
bool abstract_config_locator::load_from_system_property(std::string property_key, std::vector<std::string> accepted_suffixes){
    load_from_system_property(property_key, false, accepted_suffixes);
}
void abstract_config_locator::load_system_property_file_resource(std::string config_system_property){
    configuration_file = fopen(config_system_property.c_str(), "r");
    std::cout << "Using configuration file at " << config_system_property << std::endl;
    if(configuration_file == nullptr){
        std::string msg = "Config file at " + config_system_property + " doesn't exist.";
        throw hazelcast::client::exception::hazelcast_(msg);
    }
    in->open(config_system_property);
    if(in->fail()){
        throw hazelcast::client::exception::hazelcast_("Failed to open file: " + config_system_property);
    }

}

bool abstract_config_locator::load_from_system_property(std::string property_key,bool fail_on_unaccepted_suffix, std::vector<std::string> accepted_suffixes){
    if(accepted_suffixes.empty()){
        throw hazelcast::client::exception::illegal_argument("Parameter acceptedSuffixes must not be empty");
    }
    try {
        std::string config_system_property = System.getProperty(propertyKey);

        if (config_system_property.empty()) {
            std::cout << "FINEST: " << "Could not find " + property_key + " System property";
            return false;
        }

        if (!is_accepted_suffix_configured(config_system_property, accepted_suffixes)) {
            if (fail_on_unaccepted_suffix) {
                throw_unaccepted_suffix_in_system_property(property_key, config_system_property, accepted_suffixes);
            } else {
                return false;
            }
        }

        std::cout << "INFO: " << "Loading configuration " + config_system_property + " from System property" << property_key << std::endl;
        load_system_property_file_resource(config_system_property);
        return true;
    } catch (const hazelcast::client::exception::hazelcast_& e) {
        throw hazelcast::client::exception::hazelcast_(e.what());
    } catch (const std::runtime_error& e) {
        throw  hazelcast::client::exception::hazelcast_(e.what());
    }

}
bool abstract_config_locator::load_from_system_property_or_fail_on_unaccepted_suffix(std::string property_key, std::vector<std::string> accepted_suffixes){
    return load_from_system_property(property_key, true, accepted_suffixes);
}
bool abstract_config_locator::is_config_present(){
    return in != nullptr || configuration_file != nullptr;
}