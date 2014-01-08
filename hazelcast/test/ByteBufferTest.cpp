//
// Created by sancar koyunlu on 08/01/14.
//

#include "ByteBufferTest.h"
#include "ByteBuffer.h"
#include "iTest.h"

namespace hazelcast {
    namespace client {
        namespace test {

            void ByteBufferTest::addTests() {
                addTest(&ByteBufferTest::testBasics, "testBasics");
            }

            void ByteBufferTest::beforeClass() {

            }

            void ByteBufferTest::afterClass() {

            }

            void ByteBufferTest::beforeTest() {

            }

            void ByteBufferTest::afterTest() {

            }


            void ByteBufferTest::testBasics() {
                util::ByteBuffer buffer(100);
                buffer.writeInt(20);
                buffer.writeInt(40);
                buffer.flip();
                iTest::assertEqual(20, buffer.readInt());
                buffer.compact();
                buffer.writeInt(60);
                buffer.flip();
                iTest::assertEqual(40, buffer.readInt());
                iTest::assertEqual(60, buffer.readInt());
                buffer.clear();
            }
        }
    }
}