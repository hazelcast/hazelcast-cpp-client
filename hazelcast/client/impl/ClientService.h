#include "SerializationService.h"

#ifndef HAZELCAST_CLIENT_SERVICE
#define HAZELCAST_CLIENT_SERVICE

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        class serialization::SerializationService;

        class ClientConfig;

        namespace impl {


            class ClientService {
            public:

                ClientService(hazelcast::client::HazelcastClient& hazelcastClient);

                hazelcast::client::serialization::SerializationService& getSerializationService();

                hazelcast::client::ClientConfig& getClientConfig();

            private:
                hazelcast::client::HazelcastClient& hazelcastClient;
            };
        }
    }
}

#endif /* HAZELCAST_CLIENT_SERVICE */