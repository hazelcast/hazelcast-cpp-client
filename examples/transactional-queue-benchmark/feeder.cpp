/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/hazelcast_client.h>

int
main(int argc, char** argv)
{
    int period = 1000;

    if (argc > 1)
    {
        period = std::atoi(argv[1]);
    }

    using namespace hazelcast::client;

    auto hz = hazelcast::new_client().get();

    auto queue = hz.get_queue("jobs").get();

    int i = 0;
    while (true)
    {
        auto job = "job-" + std::to_string(i++);
        std::cout << "Put [" << job << "]" << std::endl;
        queue->put(job).get();

        char ch;
        while(ch = std::getchar())
        {
            if (ch == 112){
                std::cout << std::string(80,'=') << std::endl;

                auto items = queue->to_array<std::string>().get();

                if (items.empty())
                    std::cout << "There are no jobs." << std::endl;

                for(const std::string& entry : items)
                    std::cout << entry << std::endl;

                std::cout << std::string(80,'=') << std::endl;
            }
            else if (ch == 32){
                break;
            }
        }
    }

    return 0;
}
