#ifndef HAZELCAST_CONSTANT_COMMANDS
#define HAZELCAST_CONSTANT_COMMANDS

#include "Command.h"
#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include <iostream>
#include <stdexcept>

namespace hazelcast{
namespace client{
namespace protocol{
namespace GeneralCommands{    

static std::string NEWLINE = "\r\n";
static std::string SPACE = " ";
    
class AuthCommand : public Command{
public:
    AuthCommand(std::string name, std::string password):name(name),password(password){
        
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "AUTH";
        command += SPACE + name + SPACE + password + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());
    };
    void readHeaderLine(hazelcast::client::serialization::DataInput& dataInput){
        std::string ok; 
        ok.push_back(dataInput.readByte());    
        ok.push_back(dataInput.readByte());
        if(ok.compare("OK")){
            throw std::domain_error("wrong name or password");
        }
        
    };
    void readSizeLine(hazelcast::client::serialization::DataInput& dataInput) {
        
    };
    void readResultLine(hazelcast::client::serialization::DataInput& dataInput){
        
    };
    bool expectsResult(){
       return false;  
    };
private:
    std::string name;
    std::string password;
}; 
    
}}}}

#endif /* */