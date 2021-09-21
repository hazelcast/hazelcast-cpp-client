#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <hazelcast/hazelcastc.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class c_api_test : public ClientTestSupport {
            };

            TEST_F(c_api_test, new_client) {
                // start test server
                HazelcastServer srv(*g_srvFactory);

                int id;
                ASSERT_EQ(0, hazelcast_new_client(&id, "127.0.0.1", 5701));

                ASSERT_EQ(0, hazelcast_shutdown_client(id));
            }

            TEST_F(c_api_test, get_and_increment_counter) {
                // start test server
                HazelcastServer srv(*g_srvFactory);

                int id;
                ASSERT_EQ(0, hazelcast_new_client(&id, "127.0.0.1", 5701));

                long value;
                ASSERT_EQ(0, hazelcast_get_atomic_long_value(id, "my_counter", &value));
                ASSERT_EQ(0, value);

                ASSERT_EQ(0, hazelcast_increment_atomic_long(id, "my_counter"));

                ASSERT_EQ(0, hazelcast_get_atomic_long_value(id, "my_counter", &value));
                ASSERT_EQ(1, value);

                ASSERT_EQ(0, hazelcast_shutdown_client(id));
            }

        } // namespace test
    } // namespace client
} // namespace hazelcast
