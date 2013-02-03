#ifndef HAZELCAST_MAP_COMMANDS
#define HAZELCAST_MAP_COMMANDS

#include "../Array.h"
#include "Command.h"
#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include <iostream>
#include <stdexcept>

namespace hazelcast{
namespace client{
namespace protocol{
namespace MapCommands{    

static std::string NEWLINE = "\r\n";
static std::string SPACE = " ";
    
class PutCommand : public Command{
public:
    PutCommand(std::string instanceName, hazelcast::client::serialization::Data key , hazelcast::client::serialization::Data value):instanceName(instanceName)
                                                                                                                    ,key(key)
                                                                                                                    ,value(value)
                                                                                                                    ,returnSize(0)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MPUT";
        command += SPACE + instanceName + SPACE + "#2" + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());

        char integerBuffer[10];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(SPACE.c_str(),0,SPACE.length());

        integerBufferSize = sprintf(integerBuffer,"%d",value.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());

        key.writeData(dataOutput);
        value.writeData(dataOutput);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(hazelcast::client::serialization::DataInput& dataInput){
        std::string ok; 
        ok.push_back(dataInput.readByte());    
        ok.push_back(dataInput.readByte());
        if(ok.compare("OK")){
            throw std::domain_error("MPUT return is not OK");
        }
        if( dataInput.readByte() != ' ' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != '#' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != '1' ) throw std::domain_error("unexpected char");
        
    };
    void readSizeLine(hazelcast::client::Array<byte>& sizeInBytes) {
        returnSize = atoi((const char*)sizeInBytes.getBuffer());
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
        oldValue.readData(dataInput);
    };
    int nResults(){
       return 1;  
    };
    int resultSize(int i){
        return returnSize;
    };
    hazelcast::client::serialization::Data get(){
      return oldValue;  
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    hazelcast::client::serialization::Data value;

    hazelcast::client::serialization::Data oldValue;
    int returnSize;
}; 

class GetCommand : public Command{
public:
    GetCommand(std::string instanceName, hazelcast::client::serialization::Data key ):instanceName(instanceName)
                                                                                     ,key(key)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "1 MGET 6";
        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());

        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());

        key.writeData(dataOutput);
        
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(hazelcast::client::serialization::DataInput& dataInput){
        std::string ok; 
        if( dataInput.readByte() != '1' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != ' ' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != 'O' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != 'K' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != ' ' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != '6' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != ' ' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != '#' ) throw std::domain_error("unexpected char");
        if( dataInput.readByte() != '1' ) throw std::domain_error("unexpected char");
        
    };
    void readSizeLine(hazelcast::client::Array<byte>& sizeInBytes) {
        int returnSize = atoi((const char*)sizeInBytes.getBuffer());
        resultSizes.push_back(returnSize);
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
        oldValue.readData(dataInput);
    };
    int nResults(){
       return 1;  
    };
    int resultSize(int i){
        return resultSizes[i];//TODO do like in put command
    };
    hazelcast::client::serialization::Data get(){
      return oldValue;  
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    std::vector<int> resultSizes;
    hazelcast::client::serialization::Data oldValue;
}; 
    
}}}}

#endif /* HAZELCAST_MAP_COMMANDS */