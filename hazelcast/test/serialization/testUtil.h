#ifndef TEST_UTIL
#define TEST_UTIL

#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 5701


namespace hazelcast {
    namespace client {
        namespace test {

            class TestMainPortable;

            TestMainPortable getTestMainPortable();
        }
    }
}

#endif