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
    hazelcast::client::client_config clientConfig;

    // The default is to use port 5701 if this is not explicitely set. You can enable the below line if your hazelcast
    // server is running on another port.
    //clientConfig.setProperty(hazelcast::client::ClientProperties::PROP_AWS_MEMBER_PORT, "60000");

    // The following assumes that you provide the environment parameters AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY
    clientConfig.get_network_config().get_aws_config().set_enabled(true).
            set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(getenv("AWS_SECRET_ACCESS_KEY")).
            set_tag_key("aws-test-tag").set_tag_value("aws-tag-value-1").set_security_group_name("MySecureGroup").
            set_region("us-east-1");
    
    hazelcast::client::HazelcastClient hz(clientConfig);

    auto map = hz.get_map("MyMap");
    
    map->put(1, 100).get();
    map->put(2, 200).get();

    auto value = map->get<int, int>(1).get();

    if (value) {
        std::cout << "Value for key 1 is " << value.value() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
