/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
/**
 * This include is needed due to the Python.h include so that we do not see the linkage error:
 * unresolved external symbol __imp__invalid_parameter_noinfo_noreturn
 * It should be before the Python.h include
 */
#include <crtdefs.h>
#endif

#include <Python.h>

#include <boost/shared_ptr.hpp>

#include <ostream>

#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/Address.h>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastServerFactory {
            public:
                enum Lang {
                    JAVASCRIPT = 1,
                    GROOVY = 2,
                    PYTHON = 3,
                    RUBY = 4
                };

                struct Response {
                    bool success;
                    std::string message;
                    std::string result;
                    /*3:binary result;*/
                };

                class MemberInfo {
                public:
                    MemberInfo();

                    MemberInfo(const string &uuid, const string &ip, int port);

                    friend ostream &operator<<(ostream &os, const MemberInfo &info);

                    const string &getUuid() const;

                    Address getAddress() const;
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

                bool terminateServer(const MemberInfo &member);

                ~HazelcastServerFactory();

                static void init(const std::string &server);

                Response executeOnController(const std::string &script, Lang language);

            private:
                boost::shared_ptr<util::ILogger> logger;
                static std::string serverAddress;
                static PyObject *rcObject;
                std::string clusterId;

                std::string readFromXmlFile(const std::string &xmlFilePath);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

