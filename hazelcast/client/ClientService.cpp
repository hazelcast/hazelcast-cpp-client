#include "ClientService.h"
#include "HazelcastClient.h"
#include "serialization/SerializationService.h"
#include "protocol/CommandHandler.h"
#include "ClientConfig.h"
#include <utility>
#include <thread>


namespace hazelcast {
    namespace client {

        ClientService::ClientService(HazelcastClient& hazelcastClient) : hazelcastClient(hazelcastClient) {

        };

        ClientService::ClientService(const ClientService& rhs) : hazelcastClient(rhs.hazelcastClient) {

        };

        serialization::SerializationService& ClientService::getSerializationService() {
            return hazelcastClient.getSerializationService();
        };

        protocol::CommandHandler& ClientService::getCommandHandler() {
            return hazelcastClient.getCommandHandler();
        };

        ClientConfig& ClientService::getClientConfig() {
            return hazelcastClient.getClientConfig();
        };

    }
}