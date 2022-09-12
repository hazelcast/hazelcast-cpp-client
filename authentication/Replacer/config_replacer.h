//
// Created by Hakan Aktaş on 5.09.2022.
//
#include <hazelcast/client/hazelcast_client.h>
#ifndef HAZELCAST_CPP_CLIENT_CONFIG_REPLACER_H
#define HAZELCAST_CPP_CLIENT_CONFIG_REPLACER_H
class config_replacer{
public:
    virtual void init(hazelcast::client::client_properties properties);
    virtual std::string get_prefix();
    virtual std::string get_replacement(std::string masked_value);
};
#endif // HAZELCAST_CPP_CLIENT_CONFIG_REPLACER_H
