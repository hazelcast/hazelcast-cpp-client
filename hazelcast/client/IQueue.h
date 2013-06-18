#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include <stdexcept>

namespace hazelcast {
    namespace client {


        template<typename E>
        class IQueue {
        public:

            IQueue(const std::string& instanceName, spi::ClientContext& clientContext)
            : instanceName(instanceName)
            , context(clientContext) {

            };

            IQueue(const IQueue& rhs)
            : instanceName(rhs.instanceName)
            , context(rhs.context) {
            };

        private:
            std::string instanceName;
            spi::ClientContext& context;

        };
    }
}

#endif /* HAZELCAST_IQUEUE */