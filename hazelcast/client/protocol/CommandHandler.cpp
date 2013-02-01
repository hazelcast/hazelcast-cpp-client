#include "CommandHandler.h"
#include "Command.h"

#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"

#include <cassert>

namespace hazelcast{
namespace client{
namespace protocol{
   
CommandHandler::CommandHandler(Address address, hazelcast::client::serialization::SerializationService* serializationService)
                                :socket(address)
                                ,serializationService(serializationService)
{
    
};

CommandHandler::~CommandHandler(){
    
};

void CommandHandler::start(){
     std::string command = "P01\r\n";
     socket.sendData(command.c_str(),command.length());
};

hazelcast::client::serialization::Data CommandHandler::sendCommand(Command* const  command){
    using namespace hazelcast::client::serialization;
    DataOutput* dataOutput = serializationService->pop();
    command->writeCommand(*dataOutput);
    Array<byte> temp = dataOutput->toByteArray();
    socket.sendData(temp.getBuffer(),temp.length());

    Array<byte> headerLine = socket.readLine();
    DataInput headerInput(headerLine,serializationService);
    command->readHeaderLine(headerInput);
    
    if(command->expectsResult()){
        Array<byte> sizeLine = socket.readLine();
        command->readSizeLine(sizeLine);
        
        Array<byte> resultLine = socket.readLine();
        std::cout << "resultLine => ";
        for(int i = 0; i < resultLine.length(); i++){
            std::cout << (int)resultLine[i] ;
        }
        std::cout << std::endl;
        DataInput resultInput(resultLine,serializationService);
        command->readResultLine(resultInput);
        
        return command->returnResult();
    }
    return Data();
    
};

}}}