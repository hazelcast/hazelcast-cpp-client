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

int main() {
    hazelcast::client::ClientConfig clientConfig;

    // Your instance should be inside AWS and the instance profile should have an instance profile associated for
    // a valid IAM role. See http://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_use_switch-role-ec2_instance-profiles.html
    clientConfig.get_network_config().get_aws_config().set_enabled(true).set_tag_key("aws-test-tag").
        set_tag_value("aws-tag-value-1").set_inside_aws(true);
    
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
