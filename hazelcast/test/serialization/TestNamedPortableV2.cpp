#include "TestNamedPortableV2.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestNamedPortableV2::TestNamedPortableV2() {

            };

            TestNamedPortableV2::TestNamedPortableV2(std::string name, int v) : name(name), k(v * 10), v(v) {
            };

            bool TestNamedPortableV2::operator ==(TestNamedPortableV2 & m) {
                if (this == &m)
                    return true;
                if (k != m.k)
                    return false;
                if (name.compare(m.name))
                    return false;
                if (v != m.v) return false;
                return true;
            };

            bool TestNamedPortableV2::operator !=(TestNamedPortableV2 & m) {
                return !(*this == m);
            };


            int TestNamedPortableV2::getFactoryId() const {
                return 1;
            }

            int TestNamedPortableV2::getClassId() const {
                return 3;
            }


            void TestNamedPortableV2::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("v", v);
                writer.writeUTF("name", name);
                writer.writeInt("myint", k);
            };


            void TestNamedPortableV2::readPortable(serialization::PortableReader& reader) {
                v = reader.readInt("v");
                name = reader.readUTF("name");
                k = reader.readInt("myint");
            };

        }
    }
}