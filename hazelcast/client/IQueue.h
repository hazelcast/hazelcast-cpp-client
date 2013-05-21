#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include <stdexcept>

namespace hazelcast {
    namespace client {


        template<typename E>
        class IQueue {
        public:

            IQueue(std::string instanceName) : instanceName(instanceName) {

            };

            IQueue(const IQueue& rhs) : instanceName(rhs.instanceName) {
            };

            ~IQueue() {

            };

            std::string getName() const {
                return instanceName;
            };

        private:
            std::string instanceName;

        };
    }
}

#endif /* HAZELCAST_IQUEUE */