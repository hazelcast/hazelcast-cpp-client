#include "CommandHandler.h"
#include "Command.h"

#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"

namespace hazelcast{
namespace client{
namespace protocol{
   
CommandHandler::CommandHandler(Address address):socket(address){
    
};

CommandHandler::~CommandHandler(){
    
};
    
void CommandHandler::sendCommand(Command const * const  command){
//    hazelcast::client::serialization::DataOutput dataOutput;
//    command.writeCommand(dataOutput);
//    socket.sendData(dataOutput.toByteArray());
//    Array<byte> buffer(1024);
//    socket.recvData(buffer);
//    hazelcast::client::serialization::DataInput dataInput;
//    command.readResponse(dataInput);
    
};

}}}