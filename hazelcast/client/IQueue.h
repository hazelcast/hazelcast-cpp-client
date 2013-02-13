#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include <stdexcept>

namespace hazelcast {
    namespace client {

        class ClientService;

        template<typename E>
        class IQueue {
        public:

            IQueue(std::string instanceName, ClientService& clientService) : instanceName(instanceName)
            , clientService(clientService) {

            };

            IQueue(const IQueue& rhs) : instanceName(rhs.instanceName)
            , clientService(rhs.clientService) {
            };

            ~IQueue() {

            };

            std::string getName() const {
                return instanceName;
            };

        private:
            std::string instanceName;
            ClientService& clientService;
        };
    }
}

#endif /* HAZELCAST_IQUEUE */