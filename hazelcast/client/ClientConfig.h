#ifndef HAZELCAST_CLIENT_CONFIG
#define HAZELCAST_CLIENT_CONFIG

#include "Address.h"
#include "GroupConfig.h"
#include <list>
#include <string>

namespace hazelcast{
namespace client{

class ClientConfig{
public:
    ClientConfig();
    ClientConfig(const ClientConfig&);
    ~ClientConfig();
    GroupConfig& getGroupConfig();
    void setAddress(std::string);
    Address getAddress() const;
private:
    GroupConfig groupConfig;
    Address address;
};

}}
#endif /* HAZELCAST_CLIENT_CONFIG */