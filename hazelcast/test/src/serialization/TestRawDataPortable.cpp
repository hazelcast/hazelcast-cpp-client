/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "TestRawDataPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestRawDataPortable::TestRawDataPortable() {

            }

            int TestRawDataPortable::getFactoryId() const {
                return TestSerializationConstants::TEST_PORTABLE_FACTORY;
            }

            int TestRawDataPortable::getClassId() const {
                return TestSerializationConstants::TEST_RAW_DATA_PORTABLE;
            }


            void TestRawDataPortable::writePortable(serialization::PortableWriter &writer) const {
                writer.writeLong("l", l);
                writer.writeCharArray("c", &c);
                writer.writePortable("p", &p);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeInt(k);
                out.writeUTF(&s);
                ds.writeData(out);
            }


            void TestRawDataPortable::readPortable(serialization::PortableReader &reader) {
                l = reader.readLong("l");
                c = *reader.readCharArray("c");
                boost::shared_ptr<TestNamedPortable> ptr = reader.readPortable<TestNamedPortable>("p");
                if (ptr != NULL)
                    p = *ptr;
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                k = in.readInt();
                s = *in.readUTF();
                ds.readData(in);
            }

            TestRawDataPortable::TestRawDataPortable(int64_t l, std::vector<char> c, TestNamedPortable p, int32_t k, std::string s, TestDataSerializable ds) {
                this->l = l;
                this->c = c;
                this->p = p;
                this->k = k;
                this->s = s;
                this->ds = ds;
            }

            bool TestRawDataPortable::operator ==(const TestRawDataPortable &m) const {
                if (this == &m)
                    return true;
                if (l != m.l) return false;
                if (c != m.c) return false;
                if (p != m.p) return false;
                if (k != m.k) return false;
                if (ds != m.ds) return false;
                if (s.compare(m.s) != 0) return false;
                return true;
            }

            bool TestRawDataPortable::operator !=(const TestRawDataPortable &m) const {
                return !(*this == m);
            }
        }
    }
}

