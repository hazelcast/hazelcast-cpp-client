#include "ClientService.h"
#include "HazelcastClient.h"

using namespace hazelcast::client;

namespace hazelcast {
    namespace client {
        namespace impl {
            ClientService::ClientService(HazelcastClient& hazelcastClient) : hazelcastClient(hazelcastClient) {

            };

            serialization::SerializationService& ClientService::getSerializationService() {
                return hazelcastClient.getSerializationService();
            };


            ClientConfig& ClientService::getClientConfig() {
                return hazelcastClient.getClientConfig();
            };
        }

    }
}