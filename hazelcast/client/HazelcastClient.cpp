#include "HazelcastClient.h"
#include "protocol/GeneralCommands.h"
#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"

namespace hazelcast {
    namespace client {

        HazelcastClient::HazelcastClient(ClientConfig& config) : clientService(*this)
        , clientConfig(config)
        , serializationService(0, *clientConfig.getPortableFactories())
        , commandHandler(config.getAddress(), &serializationService) {
            std::cout << "trying to connect to " << config.getAddress().getAddress() << ":" << config.getAddress().getPort() << std::endl;
            setupInitialConnection();
            std::cout << "connected  to " << config.getAddress().getAddress() << ":" << config.getAddress().getPort() << std::endl;
        };

        HazelcastClient::~HazelcastClient() {

        }

        serialization::SerializationService& HazelcastClient::getSerializationService() {
            return serializationService;
        }

        protocol::CommandHandler& HazelcastClient::getCommandHandler() {
            return commandHandler;
        }

        ClientConfig& HazelcastClient::getClientConfig() {
            return clientConfig;
        };

        std::auto_ptr<HazelcastClient> HazelcastClient::newHazelcastClient(ClientConfig& config) {
            return std::auto_ptr<HazelcastClient >(new HazelcastClient(config));
        };

        IdGenerator HazelcastClient::getIdGenerator(std::string instanceName) {
            return IdGenerator(instanceName, clientService);
        };

        IAtomicLong HazelcastClient::getIAtomicLong(std::string instanceName) {
            return IAtomicLong(instanceName, clientService);
        };

        ICountDownLatch HazelcastClient::getICountDownLatch(std::string instanceName) {
            return ICountDownLatch(instanceName, clientService);
        };

        ISemaphore HazelcastClient::getISemaphore(std::string instanceName) {
            return ISemaphore(instanceName, clientService);
        };

        void HazelcastClient::setupInitialConnection() {
            commandHandler.start();
            protocol::GeneralCommands::AuthCommand authCommand(clientConfig.getGroupConfig().getName(), clientConfig.getGroupConfig().getPassword());
            commandHandler.sendCommand(&authCommand);

        };

    }
}