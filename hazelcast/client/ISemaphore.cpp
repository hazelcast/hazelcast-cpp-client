#include "ISemaphore.h"


namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(std::string instanceName) : instanceName(instanceName) {

        };

        ISemaphore::ISemaphore(const ISemaphore& rhs) : instanceName(rhs.instanceName) {
        };

        ISemaphore::~ISemaphore() {

        };

        std::string ISemaphore::getName() const {
            return instanceName;
        };

    }
}