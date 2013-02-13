#include "AtomicNumber.h"

namespace hazelcast {
    namespace client {

        AtomicNumber::AtomicNumber(std::string instanceName, ClientService& clientService) : instanceName(instanceName)
        , clientService(clientService) {

        };

        AtomicNumber::AtomicNumber(const AtomicNumber& rhs) : instanceName(rhs.instanceName)
        , clientService(rhs.clientService) {
        };

        AtomicNumber::~AtomicNumber() {

        };

        std::string AtomicNumber::getName() const {
            return instanceName;
        };

    }
}