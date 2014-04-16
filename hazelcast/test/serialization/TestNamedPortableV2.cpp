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
