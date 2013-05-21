#include "IAtomicLong.h"


namespace hazelcast {
    namespace client {

        IAtomicLong::IAtomicLong(std::string instanceName) : instanceName(instanceName) {

        };

        IAtomicLong::IAtomicLong(const IAtomicLong& rhs) : instanceName(rhs.instanceName) {
        };

        IAtomicLong::~IAtomicLong() {

        };

        std::string IAtomicLong::getName() const {
            return instanceName;
        };

    }
}