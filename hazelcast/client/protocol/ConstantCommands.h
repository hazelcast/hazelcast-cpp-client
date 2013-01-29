#ifndef HAZELCAST_CONSTANT_COMMANDS
#define HAZELCAST_CONSTANT_COMMANDS

#include "Command.h"
#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"

namespace hazelcast{
namespace client{
namespace protocol{
namespace ConstantCommands{    

class BeginCommand : public Command{
public:
    BeginCommand(){
        
    };
    void writeCommand(hazelcast::client::serialization::DataOutput&) {
        
    };
    void readResponse(hazelcast::client::serialization::DataInput&) {
        
    };
    
};    
    
class AuthCommand : public Command{
public:
    AuthCommand(std::string name, std::string password):name(name),password(password){
        
    };
    void writeCommand(hazelcast::client::serialization::DataOutput&) {
        
    };
    void readResponse(hazelcast::client::serialization::DataInput&) {
        
    };
private:
    std::string name;
    std::string password;
}; 
    
}}}}

#endif /* */