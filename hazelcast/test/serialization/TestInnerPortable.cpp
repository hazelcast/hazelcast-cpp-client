#include "TestInnerPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestInnerPortable::TestInnerPortable() {
            };

            TestInnerPortable::TestInnerPortable(const TestInnerPortable& rhs) {
                *this = rhs;
            }

            TestInnerPortable::TestInnerPortable(std::vector<byte> b,
                    std::vector<char> c,
                    std::vector<short> s,
                    std::vector<int> i,
                    std::vector<long> l,
                    std::vector<float> f,
                    std::vector<double> d,
                    std::vector<TestNamedPortable> n):ii(i), bb(b), cc(c), ss(s), ll(l), ff(f), dd(d), nn(n) {
            };

            TestInnerPortable::~TestInnerPortable() {
            }

            TestInnerPortable& TestInnerPortable::operator = (const TestInnerPortable& rhs) {
                bb = rhs.bb;
                cc = rhs.cc;
                ss = rhs.ss;
                ii = rhs.ii;
                ll = rhs.ll;
                ff = rhs.ff;
                dd = rhs.dd;
                nn = rhs.nn;
                return (*this);
            }

            int TestInnerPortable::getClassId() const {
                return 2;
            };

            int TestInnerPortable::getFactoryId() const {
                return 1;
            };

            bool TestInnerPortable::operator ==(const TestInnerPortable& m) const {
                if (bb != m.bb) return false;
                if (cc != m.cc) return false;
                if (ss != m.ss) return false;
                if (ii != m.ii) return false;
                if (ll != m.ll) return false;
                if (ff != m.ff) return false;
                if (dd != m.dd) return false;
                for (int i = 0; i < nn.size(); i++)
                    if (nn[i] != m.nn[i])
                        return false;
                return true;
            };


            bool TestInnerPortable::operator !=(const TestInnerPortable& m) const {
                return !(*this == m);
            };


            void TestInnerPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeByteArray("b", bb);
                writer.writeCharArray("c", cc);
                writer.writeShortArray("s", ss);
                writer.writeIntArray("i", ii);
                writer.writeLongArray("l", ll);
                writer.writeFloatArray("f", ff);
                writer.writeDoubleArray("d", dd);
                writer.writePortableArray("nn", nn);
            };

            void TestInnerPortable::readPortable(serialization::PortableReader& reader) {
                bb = reader.readByteArray("b");
                cc = reader.readCharArray("c");
                ss = reader.readShortArray("s");
                ii = reader.readIntArray("i");
                ll = reader.readLongArray("l");
                ff = reader.readFloatArray("f");
                dd = reader.readDoubleArray("d");
                nn = reader.readPortableArray<TestNamedPortable>("nn");
            };

        }
    }
}
