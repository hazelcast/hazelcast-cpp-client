/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
/**
 * You need to provide compile flag -DHZ_BUILD_WITH_SSL when compiling since Aws depends on openssl library.
 *
 * DO NOT FORGET to make sure that openssl is installed.
 *
 */
#include <hazelcast/client/HazelcastClient.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for dll export
#endif

int main() {
    hazelcast::client::ClientConfig clientConfig;

    // The default is to use port 5701 if this is not explicitely set. You can enable the below line if your hazelcast
    // server is running on another port.
    //clientConfig.getProperties()[ hazelcast::client::ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";

    // The following assumes that you provide the environment parameters AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY
    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1").setSecurityGroupName("MySecureGroup").
            setRegion("us-east-1");
    
    hazelcast::client::HazelcastClient hz(clientConfig);

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("MyMap");
    
    map.put(1, 100);
    map.put(2, 200);

    boost::shared_ptr<int> value = map.get(1);

    if (value.get()) {
        std::cout << "Value for key 1 is " << *value << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
