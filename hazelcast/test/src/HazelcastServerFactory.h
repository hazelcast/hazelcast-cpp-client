/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 8/26/13.


#ifndef HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_
#define HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

#include <Python.h>

#include <boost/shared_ptr.hpp>

#include <ostream>

using namespace std;

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace test {

            class HazelcastServerFactory {
            public:
                class MemberInfo {
                public:
                    MemberInfo();

                    MemberInfo(const string &uuid, const string &ip, int port);

                    friend ostream &operator<<(ostream &os, const MemberInfo &info);

                    const string &getUuid() const;

                private:
                    std::string uuid;
                    std::string ip;
                    int port;
                };

                HazelcastServerFactory(const std::string &serverXmlConfigFilePath);

                static const std::string& getServerAddress();

                MemberInfo startServer();

                bool setAttributes(int memberStartOrder);

                bool shutdownServer(const MemberInfo &member);

                ~HazelcastServerFactory();

                static void init(const std::string &server);

            private:
                util::ILogger &logger;
                static std::string serverAddress;
                static PyObject *rcObject;
                std::string clusterId;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

