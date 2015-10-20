#include "TestInvalidWritePortable.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestInvalidWritePortable::TestInvalidWritePortable() {

            }

            TestInvalidWritePortable::TestInvalidWritePortable(long l, int i, std::string s) {
                this->l = l;
                this->i = i;
                this->s = s;
            }

            int TestInvalidWritePortable::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestInvalidWritePortable::getClassId() const {
                return TestSerializationConstants::TEST_INVALID_WRITE_PORTABLE;
            }

            void TestInvalidWritePortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("l", l);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeInt(i);
                writer.writeUTF("s", &s);
            }

            void TestInvalidWritePortable::readPortable(serialization::PortableReader& reader) {
                l = reader.readLong("l");
                i = reader.readInt("i");
                s = *reader.readUTF("s");
            }
        }
    }
}
