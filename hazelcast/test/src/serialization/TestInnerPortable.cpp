/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "TestInnerPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestInnerPortable::TestInnerPortable() {
            }

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
            }

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
                return TestSerializationConstants::TEST_INNER_PORTABLE;
            }

            int TestInnerPortable::getFactoryId() const {
                return TestSerializationConstants::TEST_DATA_FACTORY;
            }

            bool TestInnerPortable::operator ==(const TestInnerPortable& m) const {
                if (bb != m.bb) return false;
                if (cc != m.cc) return false;
                if (ss != m.ss) return false;
                if (ii != m.ii) return false;
                if (ll != m.ll) return false;
                if (ff != m.ff) return false;
                if (dd != m.dd) return false;
                int size = nn.size();
                for (int i = 0; i < size; i++)
                    if (nn[i] != m.nn[i])
                        return false;
                return true;
            }


            bool TestInnerPortable::operator !=(const TestInnerPortable& m) const {
                return !(*this == m);
            }


            void TestInnerPortable::writePortable(serialization::PortableWriter& writer) const {
                writer.writeByteArray("b", &bb);
                writer.writeCharArray("c", &cc);
                writer.writeShortArray("s", &ss);
                writer.writeIntArray("i", &ii);
                writer.writeLongArray("l", &ll);
                writer.writeFloatArray("f", &ff);
                writer.writeDoubleArray("d", &dd);
                writer.writePortableArray("nn", &nn);
            }

            void TestInnerPortable::readPortable(serialization::PortableReader& reader) {
                bb = *reader.readByteArray("b");
                cc = *reader.readCharArray("c");
                ss = *reader.readShortArray("s");
                ii = *reader.readIntArray("i");
                ll = *reader.readLongArray("l");
                ff = *reader.readFloatArray("f");
                dd = *reader.readDoubleArray("d");
                nn = reader.readPortableArray<TestNamedPortable>("nn");
            }

        }
    }
}

