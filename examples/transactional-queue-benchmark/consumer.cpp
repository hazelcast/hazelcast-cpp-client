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

struct measure
{
    measure(std::string label)
        :   start_ {std::chrono::steady_clock::now()}
        ,   label_ {label}
    {}

    ~measure()
    {
        auto end = std::chrono::steady_clock::now();

        auto duration = end - start_;

        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        std::cout << "[" << label_ << "] took " << duration_us << "us" << std::endl;
    }

    std::chrono::steady_clock::time_point start_;
    std::string label_;
};

int
main()
{
    using namespace hazelcast::client;

    auto hz = hazelcast::new_client().get();

retry:
    transaction_options opts;
    
    opts.set_timeout(std::chrono::seconds{10});
    auto ctx = hz.new_transaction_context(
        opts
    );

    char ch;

    try
    {
        std::cout << "Before claim" << std::endl;
        while(ch = std::getchar() != 32);

        {
            measure m {"begin_transaction"};

            ctx.begin_transaction().get();
        }

        auto q = ctx.get_queue("jobs").get();

        boost::optional<std::string> value;
        
        {
            measure m {"poll"};

            do
            {
                value = q->poll<std::string>().get();
            }
            while(!value.has_value());
        }
        std::cout << "Claimed " << value << std::endl;

        while(ch = std::getchar())
        {
            if (ch == 112){ // fail
                {
                    measure m {"commit_transaction"};

                    ctx.commit_transaction().get();
                }

                std::cout << '\'' << value << "' is processed successfully." << std::endl;

                break;
            }
            else if (ch == 102){
                {
                    measure m {"rollback_transaction"};

                    ctx.rollback_transaction().get();
                }

                std::cout << '\'' << value << "' is failed." << std::endl;

                break;
            }
        }

        goto retry;
    }
    catch(const exception::iexception& e)
    {
        {
            measure m {"rollback_transaction - catch"};
            ctx.rollback_transaction().get();
        }
        std::cerr << e.what() << '\n';

        goto retry;
        exit(-1);
    }

    return 0;
}
