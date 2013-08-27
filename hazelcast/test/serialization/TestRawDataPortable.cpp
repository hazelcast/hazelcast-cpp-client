#include "TestRawDataPortable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestRawDataPortable::TestRawDataPortable() {

            };

            int TestRawDataPortable::getFactoryId() const {
                return 1;
            }

            int TestRawDataPortable::getClassId() const {
                return 4;
            }


            void TestRawDataPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("l", l);
                writer.writeCharArray("c", c);
                writer.writePortable("p", p);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeInt(k);
                out.writeUTF(s);
                ds.writeData(out);
            };


            void TestRawDataPortable::readPortable(serialization::PortableReader& reader) {
                l = reader.readLong("l");
                c = reader.readCharArray("c");
                p = reader.template readPortable<TestNamedPortable>("p");
                serialization::ObjectDataInput& in = reader.getRawDataInput();
                k = in.readInt();
                s = in.readUTF();
                ds.readData(in);
            };

            TestRawDataPortable::TestRawDataPortable(long l, std::vector<char> c, TestNamedPortable p, int k, std::string s, TestDataSerializable ds) {
                this->l = l;
                this->c = c;
                this->p = p;
                this->k = k;
                this->s = s;
                this->ds = ds;
            };

            bool TestRawDataPortable::operator ==(const TestRawDataPortable& m) const {
                if (this == &m)
                    return true;
                if (l != m.l) return false;
                if (c != m.c) return false;
                if (p != m.p) return false;
                if (k != m.k) return false;
                if (ds != m.ds) return false;
                if (s.compare(m.s) != 0) return false;
                return true;
            };

            bool TestRawDataPortable::operator !=(const TestRawDataPortable& m) const {
                return !(*this == m);
            };
        }
    }
}