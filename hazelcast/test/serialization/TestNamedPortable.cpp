#include "TestNamedPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {

            TestNamedPortable::TestNamedPortable() {
            }

            TestNamedPortable::TestNamedPortable(std::string name, int k):name(name), k(k) {
            }

            int TestNamedPortable::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestNamedPortable::getClassId() const {
                return TestSerializationConstants::TEST_NAMED_PORTABLE;
            }

            void TestNamedPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", name);
                writer.writeInt("myint", k);
            }


            void TestNamedPortable::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("name");
                k = reader.readInt("myint");
            }

            bool TestNamedPortable::operator ==(const TestNamedPortable& m) const {
                if (this == &m)
                    return true;
                if (k != m.k)
                    return false;
                if (name.compare(m.name))
                    return false;
                return true;
            }

            bool TestNamedPortable::operator !=(const TestNamedPortable& m) const {
                return !(*this == m);
            }

        }
    }
}
