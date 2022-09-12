//
// Created by Hakan Aktaş on 5.09.2022.
//

#ifndef HAZELCAST_CPP_CLIENT_PROPERTY_REPLACER_H
#define HAZELCAST_CPP_CLIENT_PROPERTY_REPLACER_H

#include "config_replacer.h"
class property_replacer : public config_replacer
{
private:
    hazelcast::client::client_properties properties;
public:
    property_replacer();
    void init(hazelcast::client::client_properties properties) override;
    std::string get_prefix() override;
    std::string get_replacement(std::string variable) override;
};

#endif // HAZELCAST_CPP_CLIENT_PROPERTY_REPLACER_H
