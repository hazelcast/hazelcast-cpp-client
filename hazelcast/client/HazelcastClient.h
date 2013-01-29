#ifndef HAZELCAST_CLIENT
#define HAZELCAST_CLIENT

#include "protocol/CommandHandler.h"
#include <memory>

namespace hazelcast{
namespace client{

class ClientConfig;

class HazelcastClient{
public:
    static std::auto_ptr<HazelcastClient> newHazelcastClient(ClientConfig& config);
    ~HazelcastClient();
private:
    HazelcastClient(ClientConfig&);
    HazelcastClient(const HazelcastClient&);
    protocol::CommandHandler commandHandler;
};

}}
#endif /* HAZELCAST_CLIENT */