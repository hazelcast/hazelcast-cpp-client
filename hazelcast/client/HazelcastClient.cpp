#include "HazelcastClient.h"
#include "IdGenerator.h"
#include "IAtomicLong.h"
#include "ICountDownLatch.h"
#include "ISemaphore.h"

namespace hazelcast {
    namespace client {

        HazelcastClient::HazelcastClient(ClientConfig& config)
        :clientConfig(config)
        , serializationService(0) {
        };


        HazelcastClient::HazelcastClient(HazelcastClient const & rhs)
        : clientConfig(rhs.clientConfig)
        , serializationService (0) {

        }

        HazelcastClient::~HazelcastClient() {

        }

        serialization::SerializationService& HazelcastClient::getSerializationService() {
            return serializationService;
        }


        ClientConfig& HazelcastClient::getClientConfig() {
            return clientConfig;
        };

        IdGenerator HazelcastClient::getIdGenerator(std::string instanceName) {
            return IdGenerator(instanceName);
        };

        IAtomicLong HazelcastClient::getIAtomicLong(std::string instanceName) {
            return IAtomicLong(instanceName);
        };

        ICountDownLatch HazelcastClient::getICountDownLatch(std::string instanceName) {
            return ICountDownLatch(instanceName);
        };

        ISemaphore HazelcastClient::getISemaphore(std::string instanceName) {
            return ISemaphore(instanceName);
        };
    }
}