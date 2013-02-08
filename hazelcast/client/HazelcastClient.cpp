#include "HazelcastClient.h"
#include "protocol/GeneralCommands.h"
#include "protocol/CommandHandler.h"
#include "ClientConfig.h"
#include <memory>
#include <iterator>
#include <iostream>

namespace hazelcast{
namespace client{
    
HazelcastClient::HazelcastClient(ClientConfig& config):clientConfig(config)
                                                      ,serializationService(0,clientConfig.getPortableFactory())
                                                      ,commandHandler(config.getAddress(),&serializationService)
{
    std::cout << "trying to connect to " << config.getAddress().getAddress() << ":" << config.getAddress().getPort() << std::endl;
    setupInitialConnection();
    std::cout << "connected  to " << config.getAddress().getAddress() << ":" << config.getAddress().getPort() << std::endl;    
};


HazelcastClient::~HazelcastClient(){
    
}

serialization::SerializationService& HazelcastClient::getSerializationService(){
    return serializationService;
}

protocol::CommandHandler& HazelcastClient::getCommandHandler(){
    return commandHandler;
}

ClientConfig& HazelcastClient::getClientConfig(){
    return clientConfig;
};
  
std::auto_ptr<HazelcastClient> HazelcastClient::newHazelcastClient(ClientConfig& config){
    return std::auto_ptr<HazelcastClient>(new HazelcastClient(config) );
};

void HazelcastClient::setupInitialConnection(){
    commandHandler.start();
    protocol::GeneralCommands::AuthCommand authCommand(clientConfig.getGroupConfig().getName(), clientConfig.getGroupConfig().getPassword() );
    commandHandler.sendCommand(&authCommand);   
    
};

}}