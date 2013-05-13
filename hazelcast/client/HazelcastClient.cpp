//#include "HazelcastClient.h"
//#include "IdGenerator.h"
//#include "IAtomicLong.h"
//#include "ICountDownLatch.h"
//#include "ISemaphore.h"
//
//namespace hazelcast {
//    namespace client {
//
//        HazelcastClient::HazelcastClient(ClientConfig& config)
//        :clientConfig(config)
//        , serializationService(0)
//        , clientService(*this) {
//            std::cout << "trying to connect to " << config.getAddress().getAddress() << ":" << config.getAddress().getPort() << std::endl;
//            setupInitialConnection();
//            std::cout << "connected  to " << config.getAddress().getAddress() << ":" << config.getAddress().getPort() << std::endl;
//        };
//
//
//        HazelcastClient::HazelcastClient(HazelcastClient const & rhs)
//        : clientService(*this)
//        , clientConfig(rhs.clientConfig)
//        , serializationService (0) {
//
//        }
//
//        HazelcastClient::~HazelcastClient() {
//
//        }
//
//        serialization::SerializationService& HazelcastClient::getSerializationService() {
//            return serializationService;
//        }
//
//
//        ClientConfig& HazelcastClient::getClientConfig() {
//            return clientConfig;
//        };
//
//        IdGenerator HazelcastClient::getIdGenerator(std::string instanceName) {
//            return IdGenerator(instanceName, clientService);
//        };
//
//        IAtomicLong HazelcastClient::getIAtomicLong(std::string instanceName) {
//            return IAtomicLong(instanceName, clientService);
//        };
//
//        ICountDownLatch HazelcastClient::getICountDownLatch(std::string instanceName) {
//            return ICountDownLatch(instanceName, clientService);
//        };
//
//        ISemaphore HazelcastClient::getISemaphore(std::string instanceName) {
//            return ISemaphore(instanceName, clientService);
//        };
//
//        void HazelcastClient::setupInitialConnection() {
//
//        };
//
//    }
//}