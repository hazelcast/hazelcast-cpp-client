#include "TestNamedPortable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {

            TestNamedPortable::TestNamedPortable() {
            };

            TestNamedPortable::TestNamedPortable(std::string name, int k):name(name), k(k) {
            };

            int TestNamedPortable::getFactoryId() const {
                return 1;
            }

            int TestNamedPortable::getClassId() const {
                return 3;
            }

            void TestNamedPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", name);
                writer.writeInt("myint", k);
            };


            void TestNamedPortable::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("name");
                k = reader.readInt("myint");
            };

            bool TestNamedPortable::operator ==(const TestNamedPortable& m) const {
                if (this == &m)
                    return true;
                if (k != m.k)
                    return false;
                if (name.compare(m.name))
                    return false;
                return true;
            };

            bool TestNamedPortable::operator !=(const TestNamedPortable& m) const {
                return !(*this == m);
            };

        }
    }
}