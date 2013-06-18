#include "ISemaphore.h"


namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string& instanceName, spi::ClientContext& clientContext)
        : instanceName(instanceName)
        , context(clientContext) {

        };

    }
}