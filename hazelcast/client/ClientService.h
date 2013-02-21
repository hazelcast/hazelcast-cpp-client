#ifndef HAZELCAST_CLIENT_SERVICE
#define HAZELCAST_CLIENT_SERVICE

#include "serialization/SerializationService.h"
#include "protocol/CommandHandler.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;
        class serialization::SerializationService;
        class protocol::CommandHandler;
        class ClientConfig;

        class ClientService {
        public:

            ClientService(HazelcastClient& hazelcastClient);

            serialization::SerializationService& getSerializationService();

            protocol::CommandHandler& getCommandHandler();

            ClientConfig& getClientConfig();

        private:
            HazelcastClient& hazelcastClient;
        };
    }
}

#endif /* HAZELCAST_CLIENT_SERVICE */