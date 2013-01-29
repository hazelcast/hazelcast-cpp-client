#include "HazelcastClient.h"
#include "protocol/ConstantCommands.h"
#include "protocol/CommandHandler.h"
#include "ClientConfig.h"
#include <memory>
#include <iterator>
#include <iostream>

namespace hazelcast{
namespace client{
    
HazelcastClient::HazelcastClient(ClientConfig& config):commandHandler(config.getAddress()){
    protocol::ConstantCommands::BeginCommand* beginCommand = new protocol::ConstantCommands::BeginCommand;
    commandHandler.sendCommand(beginCommand);
    delete beginCommand;
    protocol::ConstantCommands::AuthCommand* authCommand = new protocol::ConstantCommands::AuthCommand(config.getGroupConfig().getName(), config.getGroupConfig().getPassword() );
    commandHandler.sendCommand(authCommand);
    delete authCommand;
};

HazelcastClient::~HazelcastClient(){
    //intentionally left empty
};
  
std::auto_ptr<HazelcastClient> HazelcastClient::newHazelcastClient(ClientConfig& config){
    return std::auto_ptr<HazelcastClient>(new HazelcastClient(config) );
};

}}