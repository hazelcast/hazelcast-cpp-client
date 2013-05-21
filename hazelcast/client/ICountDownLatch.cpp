#include "ICountDownLatch.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(std::string instanceName) : instanceName(instanceName) {

        };

        ICountDownLatch::ICountDownLatch(const ICountDownLatch& rhs) : instanceName(rhs.instanceName) {
        };

        ICountDownLatch::~ICountDownLatch() {

        };

        std::string ICountDownLatch::getName() const {
            return instanceName;
        };

    }
}