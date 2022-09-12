//
// Created by Hakan Aktaş on 4.09.2022.
//

#include "xml_client_config_locator.h"
bool xml_client_config_locator::locate_from_system_property_or_fail_on_unaccepted_suffix(){
    return load_from_system_property_or_fail_on_unaccepted_suffix(declarative_config_util::SYSPROP_CLIENT_CONFIG, declarative_config_util::XML_ACCEPTED_SUFFIXES);
}
bool xml_client_config_locator::locate_in_work_directory(){
    return load_from_working_directory("hazelcast-client.xml");
}
bool xml_client_config_locator::locate_from_system_property(){
    return load_from_system_property(declarative_config_util::SYSPROP_CLIENT_CONFIG, declarative_config_util::XML_ACCEPTED_SUFFIXES);
}