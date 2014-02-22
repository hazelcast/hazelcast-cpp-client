#include "TestInvalidReadPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestInvalidReadPortable::TestInvalidReadPortable() {

            }

            TestInvalidReadPortable::TestInvalidReadPortable(long l, int i, std::string s) {
                this->l = l;
                this->i = i;
                this->s = l;
            }

            int TestInvalidReadPortable::getFactoryId() const {
                return 1;
            }

            int TestInvalidReadPortable::getClassId() const {
                return 6;
            }

            void TestInvalidReadPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("l", l);
                writer.writeInt("i", i);
                writer.writeUTF("s", s);
            };


            void TestInvalidReadPortable::readPortable(serialization::PortableReader& reader) {
                l = reader.readLong("l");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                i = in.readInt();
                s = reader.readLong("s");
            };
        }
    }
}