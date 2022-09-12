//
// Created by Hakan Aktaş on 31.08.2022.
//

#include "abstract_config_builder.h"
auto VALIDATION_ENABLED_PROP = new hazelcast::client::client_property("hazelcast.config.schema.validation.enabled","true");

bool should_validate_the_schema(){
    hazelcast::client::client_properties hazelcast::client::client_properties::get_boolean(VALIDATION_ENABLED_PROP)
    return
}
