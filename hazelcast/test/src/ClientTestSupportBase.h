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
//
// Created by İhsan Demir on 26/05/15.
//

#ifndef HAZELCAST_CLIENT_TEST_CLIENTTESTSUPPORTBASE_H
#define HAZELCAST_CLIENT_TEST_CLIENTTESTSUPPORTBASE_H

#include <memory>
#include <string>
#include <stdint.h>

#include <TestHelperFunctions.h>
#include <hazelcast/util/Runnable.h>
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/Member.h>
#include <hazelcast/client/spi/ClientContext.h>

namespace hazelcast {
    namespace client {
        class ClientConfig;

        class HazelcastClient;

        namespace test {
            class HazelcastServerFactory;

            extern HazelcastServerFactory *g_srvFactory;

            class ClientTestSupportBase {
            public:
                ClientTestSupportBase();

                static std::string getCAFilePath();

                static std::string randomMapName();

                static std::string randomString();

                static void sleepSeconds(int32_t seconds);

                static std::string generateKeyOwnedBy(spi::ClientContext &context, const Member &member);
            protected:

                static std::auto_ptr<hazelcast::client::ClientConfig> getConfig();

                static std::auto_ptr<HazelcastClient> getNewClient();

                static const std::string getSslFilePath();

            };
        }
    }

    namespace util {
        class Thread;

        class ThreadArgs {
        public:
            const void *arg0;
            const void *arg1;
            const void *arg2;
            const void *arg3;
            util::Thread *currentThread;

            void (*func)(ThreadArgs &);
        };


        class StartedThread : util::Runnable {
        public:
            StartedThread(const std::string &name, void (*func)(ThreadArgs &),
                          void *arg0 = NULL, void *arg1 = NULL, void *arg2 = NULL, void *arg3 = NULL);

            StartedThread(void (func)(ThreadArgs &),
                          void *arg0 = NULL,
                          void *arg1 = NULL,
                          void *arg2 = NULL,
                          void *arg3 = NULL);

            virtual ~StartedThread();

            bool join();

            void cancel();

            virtual void run();

            virtual const std::string getName() const;

        private:
            ThreadArgs threadArgs;
            std::string name;
            std::auto_ptr<util::Thread> thread;
            boost::shared_ptr<util::ILogger> logger;

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3);
        };
    }
}


#endif //HAZELCAST_CLIENT_TEST_CLIENTTESTSUPPORTBASE_H
