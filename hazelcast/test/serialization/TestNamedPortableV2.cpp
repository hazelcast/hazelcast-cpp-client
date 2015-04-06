#include "TestNamedPortableV2.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestNamedPortableV2::TestNamedPortableV2() {

            }

            TestNamedPortableV2::TestNamedPortableV2(std::string name, int v) : name(name), k(v * 10), v(v) {
            }

            int TestNamedPortableV2::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestNamedPortableV2::getClassId() const {
                return TestSerializationConstants::TEST_NAMED_PORTABLE_2;
            }


            void TestNamedPortableV2::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("v", v);
                writer.writeUTF("name", name);
                writer.writeInt("myint", k);
            }


            void TestNamedPortableV2::readPortable(serialization::PortableReader& reader) {
                v = reader.readInt("v");
                name = reader.readUTF("name");
                k = reader.readInt("myint");
            }

        }
    }
}
