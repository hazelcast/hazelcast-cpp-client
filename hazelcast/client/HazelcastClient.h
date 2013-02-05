#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "protocol/CommandHandler.h"
#include "serialization/SerializationService.h"
#include "ClientConfig.h"
#include "IMap.h"
#include <memory>
#include <map>

namespace hazelcast{
namespace client{

class ClientConfig;

class HazelcastClient{
public:
    static std::auto_ptr<HazelcastClient> newHazelcastClient(ClientConfig& config);
    
    template<typename K, typename V>
    IMap<K,V> getMap(std::string instanceName){
        return IMap<K,V>(instanceName,*this);
    };
    
    ~HazelcastClient();
    serialization::SerializationService& getSerializationService();
    protocol::CommandHandler& getCommandHandler();
    ClientConfig& getClientConfig();

private:    
    void setupInitialConnection();
    
    HazelcastClient(ClientConfig&);
    HazelcastClient(const HazelcastClient&);

    ClientConfig clientConfig;
    protocol::CommandHandler commandHandler;
    serialization::SerializationService serializationService;
};

}}
#endif /* HAZELCAST_CLIENT */