#include "ICountDownLatch.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string& instanceName, spi::ClientContext& clientContext)
        : instanceName(instanceName)
        , context(clientContext) {

        };


    }
}