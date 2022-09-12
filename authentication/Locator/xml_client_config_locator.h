//
// Created by Hakan Aktaş on 4.09.2022.
//
#include "abstract_config_locator.h"
#include "declarative_config_util.h"
#ifndef HAZELCAST_CPP_CLIENT_XML_CLIENT_CONFIG_LOCATOR_H
#define HAZELCAST_CPP_CLIENT_XML_CLIENT_CONFIG_LOCATOR_H

class xml_client_config_locator : public abstract_config_locator
{
protected:
    bool locate_from_system_property_or_fail_on_unaccepted_suffix() override;
    bool locate_in_work_directory() override;
public:
    bool locate_from_system_property() override;
};

#endif // HAZELCAST_CPP_CLIENT_XML_CLIENT_CONFIG_LOCATOR_H
