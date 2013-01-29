#ifndef HAZELCAST_COMMAND_HANDLER
#define HAZELCAST_COMMAND_HANDLER

#include "Socket.h"

namespace hazelcast{
namespace client{
namespace protocol{

class Command;   
class Socket;
    
class CommandHandler{
public:
    CommandHandler(Address address);
    void sendCommand(Command const * const  command);
    ~CommandHandler();
    
private:
    Socket socket;
    CommandHandler(const CommandHandler& );
};    
    
}}}

#endif /* HAZELCAST_COMMAND_HANDLER */