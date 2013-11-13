#include "TestMainPortable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestMainPortable::TestMainPortable()
            :null(true) {
            };

            TestMainPortable::TestMainPortable(const TestMainPortable& rhs) {
                *this = rhs;
            }

            TestMainPortable::~TestMainPortable() {
            }

            TestMainPortable::TestMainPortable(byte b, bool boolean, char c, short s, int i, long l, float f, double d, std::string str, TestInnerPortable p) {
                null = false;
                this->b = b;
                this->boolean = boolean;
                this->c = c;
                this->s = s;
                this->i = i;
                this->l = l;
                this->f = f;
                this->d = d;
                this->str = str;
                this->p = p;
            };

            const TestMainPortable& TestMainPortable::operator = (const TestMainPortable& rhs) {
                null = rhs.null;
                b = rhs.b;
                boolean = rhs.boolean;
                c = rhs.c;
                s = rhs.s;
                i = rhs.i;
                l = rhs.l;
                f = rhs.f;
                d = rhs.d;
                str = rhs.str;
                p = rhs.p;
                return (*this);
            };


            bool TestMainPortable::operator ==(const TestMainPortable& m) const {
                if (this == &m) return true;
                if (null == true && m.null == true)
                    return true;
                if (b != m.b) return false;
                if (boolean != m.boolean) return false;
                if (c != m.c) return false;
                if (s != m.s) return false;
                if (i != m.i) return false;
                if (l != m.l) return false;
                if (f != m.f) return false;
                if (d != m.d) return false;
                if (str.compare(m.str)) return false;
                if (p != m.p) return false;
                return true;
            };

            bool TestMainPortable::operator !=(const TestMainPortable& m) const {
                return !(*this == m);
            };

            int TestMainPortable::getFactoryId() const {
                return 1;
            }

            int TestMainPortable::getClassId() const {
                return 1;
            }

            void TestMainPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeByte("b", b);
                writer.writeBoolean("bool", boolean);
                writer.writeChar("c", c);
                writer.writeShort("s", s);
                writer.writeInt("i", i);
                writer.writeLong("l", l);
                writer.writeFloat("f", f);
                writer.writeDouble("d", d);
                writer.writeUTF("str", str);
                writer.writePortable("p", p);
            };


            void TestMainPortable::readPortable(serialization::PortableReader& reader) {
                null = false;
                b = reader.readByte("b");
                boolean = reader.readBoolean("bool");
                c = reader.readChar("c");
                s = reader.readShort("s");
                i = reader.readInt("i");
                l = reader.readLong("l");
                f = reader.readFloat("f");
                d = reader.readDouble("d");
                str = reader.readUTF("str");
                p = reader.readPortable<TestInnerPortable>("p");
            };

        }
    }
}
