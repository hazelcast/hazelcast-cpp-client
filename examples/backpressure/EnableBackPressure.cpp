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

#include <hazelcast/client/HazelcastClient.h>

class WaitMultiplierProcessor : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    virtual int getFactoryId() const {
        return 666;
    }

    virtual int getClassId() const {
        return 8;
    }

    virtual void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeInt(INT32_MAX);
        out.writeInt(5);
    }

    virtual void readData(hazelcast::client::serialization::ObjectDataInput &reader) {
        // no-op. Client never needs to read
    }
};

int main() {
    hazelcast::client::ClientConfig config;

    /**
     * The maximum number of concurrent invocations allowed.
     * <p/>
     * To prevent the system from overloading, user can apply a constraint on the number of concurrent invocations.
     * If the maximum number of concurrent invocations has been exceeded and a new invocation comes in,
     * then hazelcast will throw HazelcastOverloadException
     * <p/>
     * By default it is configured as INT32_MAX.
     *
     * The following sets the maximum allowable outstanding concurrent invocations to 10000.
     */
    config.setProperty("hazelcast.client.max.concurrent.invocations", "5");

    /**
     * Control the maximum timeout in millis to wait for an invocation space to be available.
     * <p/>
     * If an invocation can't be made because there are too many pending invocations, then an exponential backoff is done
     * to give the system time to deal with the backlog of invocations. This property controls how long an invocation is
     * allowed to wait before getting a {@link com.hazelcast.core.HazelcastOverloadException}.
     * <p/>
     * <p>
     * When set to -1 then <code>HazelcastOverloadException</code> is thrown immediately without any waiting. This is
     * the default value.
     * </p>
     *
     * The following lets a maximum of 2 seconds wait time before throwing the exception.
     */
    config.setProperty("hazelcast.client.invocation.backoff.timeout.millis", "2000");

    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("MyMap");
    
    map.put(1, 1);

    WaitMultiplierProcessor blockerTask;

    // submit 5 blocker tasks
    map.submitToKey<int, WaitMultiplierProcessor>(1, blockerTask);
    map.submitToKey<int, WaitMultiplierProcessor>(1, blockerTask);
    map.submitToKey<int, WaitMultiplierProcessor>(1, blockerTask);
    map.submitToKey<int, WaitMultiplierProcessor>(1, blockerTask);
    map.submitToKey<int, WaitMultiplierProcessor>(1, blockerTask);

    // Now the 6th call should receive HazelcastOverloadException
    try {
        map.submitToKey<int, WaitMultiplierProcessor>(1, blockerTask);
        std::cout << "This line should not execute!!!" << std::endl;
    } catch (hazelcast::client::exception::HazelcastOverloadException &) {
        std::cout << "Received the expected overload exception." << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
