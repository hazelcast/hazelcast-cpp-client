#include "IdGenerator.h"

namespace hazelcast {
    namespace client {

        IdGenerator::IdGenerator(const std::string& instanceName, spi::ClientContext& clientContext)
        : instanceName(instanceName)
        , context(clientContext) {

        };

        bool IdGenerator::init(long id) {
            return 0;
        };

        long IdGenerator::newId() {
            return 0;
        };
    }
}