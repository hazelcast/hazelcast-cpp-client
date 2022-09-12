//
// Created by Hakan Aktaş on 3.09.2022.
//
#include <hazelcast/client/hazelcast_client.h>
#include "declarative_config_util.h"
#include <fstream>
#include <boost/filesystem.hpp>
#ifndef HAZELCAST_CPP_CLIENT_ABSTRACT_CONFIG_LOCATOR_H
#define HAZELCAST_CPP_CLIENT_ABSTRACT_CONFIG_LOCATOR_H

class abstract_config_locator
{
private:
    std::ifstream* in;
    std::FILE * configuration_file;
    bool load_from_system_property(std::string property_key, bool fail_on_unaccepted_suffix, std::vector<std::string> accepted_suffixes);
    void load_system_property_file_resource(std::string config_system_property);

protected:
    bool load_from_working_directory(std::string config_file_path);
    bool load_from_working_directory(std::string config_file_prefix, std::vector<std::string> accepted_suffixes);
    bool load_from_system_property(std::string property_key, std::vector<std::string> accepted_suffixes);
    bool load_from_system_property_or_fail_on_unaccepted_suffix(std::string property_key, std::vector<std::string> accepted_suffixes);
public:
    std::ifstream* get_in(){
        return in;
    }
    std::FILE* get_configuration_file(){
        return configuration_file;
    };
    bool is_config_present();
    virtual bool locate_from_system_property();
    virtual bool locate_from_system_property_or_fail_on_unaccepted_suffix();
    virtual bool locate_in_work_directory();
    virtual bool locate_default();
    bool locate_everywhere();
};

#endif // HAZELCAST_CPP_CLIENT_ABSTRACT_CONFIG_LOCATOR_H
