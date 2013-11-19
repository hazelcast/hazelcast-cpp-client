#include "TestDataSerializable.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestDataSerializable::TestDataSerializable() {

            }

            TestDataSerializable::TestDataSerializable(int i, char c):i(i), c(c) {

            }

            bool TestDataSerializable::operator ==(const TestDataSerializable & rhs) const {
                if (this == &rhs)
                    return true;
                if (i != rhs.i) return false;
                if (c != rhs.c) return false;
                return true;
            };

            bool TestDataSerializable::operator !=(const TestDataSerializable& m) const {
                return !(*this == m);
            };

            int TestDataSerializable::getFactoryId() const {
                return 1;
            }

            int TestDataSerializable::getClassId() const {
                return 1;
            }

            void TestDataSerializable::writeData(serialization::ObjectDataOutput& writer) const {
                writer.writeChar(c);
                writer.writeInt(i);
            };

            void TestDataSerializable::readData(serialization::ObjectDataInput& reader) {
                c = reader.readChar();
                i = reader.readInt();
            };
        }
    }
}


