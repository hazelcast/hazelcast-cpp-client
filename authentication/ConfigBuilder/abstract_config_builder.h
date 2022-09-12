//
// Created by Hakan Aktaş on 31.08.2022.
//
#include "hazelcast/client/hazelcast_client.h"
#include <boost/property_tree/ptree.hpp>

#ifndef HAZELCAST_CPP_CLIENT_ABSTRACT_CONFIG_BUILDER_H
#define HAZELCAST_CPP_CLIENT_ABSTRACT_CONFIG_BUILDER_H

class abstract_config_builder
{
private:
    const static hazelcast::client::client_property* VALIDATION_ENABLED_PROP;
protected:
    bool should_validate_the_schema();
};

#endif // HAZELCAST_CPP_CLIENT_ABSTRACT_CONFIG_BUILDER_H
