#include "customSerialization/TestCustomXSerializable.h"

namespace hazelcast {
    namespace client {
        namespace test {
            int TestCustomXSerializable::getSerializerId() const {
                return 666;
            };

            std::string TestCustomPerson::getName() const {
                return name;
            };

            void TestCustomPerson::setName(const std::string & param) {
                name = param;
            };

            int TestCustomPerson::getSerializerId() const {
                return 999;
            };

        }
    }
}

