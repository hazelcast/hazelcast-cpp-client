#include "IAtomicLong.h"


namespace hazelcast {
    namespace client {

        IAtomicLong::IAtomicLong(const std::string& instanceName, spi::ClientContext& clientContext)
        : instanceName(instanceName), context(clientContext) {

        };


    }
}