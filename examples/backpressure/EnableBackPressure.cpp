/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <stdint.h>

#include <hazelcast/client/hazelcast_client.h>

class WaitMultiplierProcessor {};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<WaitMultiplierProcessor> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 666;
                }

                static int32_t get_class_id() noexcept {
                    return 8;
                }

                static void write_data(const WaitMultiplierProcessor &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(INT32_MAX);
                    out.write(5);
                }

                static WaitMultiplierProcessor read_data(hazelcast::client::serialization::object_data_input &in) {
                    return WaitMultiplierProcessor{};
                }
            };
        }
    }
}

int main() {
    hazelcast::client::client_config config;

    /**
     * The maximum number of concurrent invocations allowed.
     * <p/>
     * To prevent the system from overloading, user can apply a constraint on the number of concurrent invocations.
     * If the maximum number of concurrent invocations has been exceeded and a new invocation comes in,
     * then hazelcast will throw hazelcast_overload
     * <p/>
     * By default it is configured as INT32_MAX.
     *
     * The following sets the maximum allowable outstanding concurrent invocations to 10000.
     */
    config.set_property("hazelcast.client.max.concurrent.invocations", "5");

    /**
     * Control the maximum timeout in millis to wait for an invocation space to be available.
     * <p/>
     * If an invocation can't be made because there are too many pending invocations, then an exponential backoff is done
     * to give the system time to deal with the backlog of invocations. This property controls how long an invocation is
     * allowed to wait before getting a {@link com.hazelcast.core.hazelcast_overload}.
     * <p/>
     * <p>
     * When set to -1 then <code>hazelcast_overload</code> is thrown immediately without any waiting. This is
     * the default value.
     * </p>
     *
     * The following lets a maximum of 2 seconds wait time before throwing the exception.
     */
    config.set_property("hazelcast.client.invocation.backoff.timeout.millis", "2000");

    hazelcast::client::hazelcast_client hz(config);

    auto map = hz.get_map("MyMap");
    
    map->put(1, 1).get();

    WaitMultiplierProcessor blockerTask;

    // submit 5 blocker tasks
    map->submit_to_key<int, WaitMultiplierProcessor>(1, blockerTask);
    map->submit_to_key<int, WaitMultiplierProcessor>(1, blockerTask);
    map->submit_to_key<int, WaitMultiplierProcessor>(1, blockerTask);
    map->submit_to_key<int, WaitMultiplierProcessor>(1, blockerTask);
    map->submit_to_key<int, WaitMultiplierProcessor>(1, blockerTask);

    // Now the 6th call should receive hazelcast_overload
    try {
        map->submit_to_key<int, WaitMultiplierProcessor>(1, blockerTask);
        std::cout << "This line should not execute!!!" << std::endl;
    } catch (hazelcast::client::exception::hazelcast_overload &) {
        std::cout << "Received the expected overload exception." << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
