//
// Created by sancar koyunlu on 08/01/14.
//


#ifndef HAZELCAST_ByteBufferTest
#define HAZELCAST_ByteBufferTest

#include "iTest/iTestFixture.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ByteBufferTest : public iTest::iTestFixture<ByteBufferTest> {

            public:
                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testBasics();
            };
        }
    }
}


#endif //HAZELCAST_ByteBufferTest
