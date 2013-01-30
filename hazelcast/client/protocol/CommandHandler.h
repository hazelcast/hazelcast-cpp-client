#ifndef HAZELCAST_COMMAND_HANDLER
#define HAZELCAST_COMMAND_HANDLER

#include "Socket.h"
#include "../serialization/SerializationService.h"

namespace hazelcast{
namespace client{
namespace protocol{

class Command;   
class Socket;
    
class CommandHandler{
public:
    CommandHandler(Address address,hazelcast::client::serialization::SerializationService* serializationService);
    void start();
    void sendCommand(Command* const  command);
    ~CommandHandler();
    
private:
    hazelcast::client::serialization::SerializationService* serializationService;
    Socket socket;
    CommandHandler(const CommandHandler& );
    bool readResponseHeader();
};    
    
}}}

#endif /* HAZELCAST_COMMAND_HANDLER */