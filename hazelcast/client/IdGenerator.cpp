#include "IdGenerator.h"

namespace hazelcast {
    namespace client {

        IdGenerator::IdGenerator(std::string instanceName, ClientService& clientService) : instanceName(instanceName)
        , clientService(clientService) {

        };

        IdGenerator::IdGenerator(const IdGenerator& rhs) : instanceName(rhs.instanceName)
        , clientService(rhs.clientService) {
        };

        IdGenerator::~IdGenerator() {

        };

        std::string IdGenerator::getName() const {
            return instanceName;
        };

        bool IdGenerator::init(long id) {
            return 0;
        };

        long IdGenerator::newId() {
            return 0;
        };
    }
}