#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "protocol/CommandHandler.h"
#include "serialization/SerializationService.h"
#include "ClientConfig.h"
#include <memory>

namespace hazelcast{
namespace client{

class ClientConfig;

class HazelcastClient{
public:
    static std::auto_ptr<HazelcastClient> newHazelcastClient(ClientConfig& config);
    ~HazelcastClient();
private:
    ClientConfig clientConfig;
    protocol::CommandHandler commandHandler;
    serialization::SerializationService serializationService;
    
    void setupInitialConnection();
    
    HazelcastClient(ClientConfig&);
    HazelcastClient(const HazelcastClient&);
};

}}
#endif /* HAZELCAST_CLIENT */