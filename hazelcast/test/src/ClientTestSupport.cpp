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

#include <hazelcast/client/exception/IllegalStateException.h>
#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {
        namespace test {
            ClientTestSupport::ClientTestSupport() {
                const testing::TestInfo *testInfo = testing::UnitTest::GetInstance()->current_test_info();
                std::ostringstream out;
                out << testInfo->test_case_name() << "_" << testInfo->name();
                testName = out.str();
                logger.reset(new util::ILogger("Test", testName, "TestVersion", config::LoggerConfig()));
                if (!logger->start()) {
                    throw (exception::ExceptionBuilder<exception::IllegalStateException>(
                            "ClientTestSupport::ClientTestSupport()") << "Could not start logger "
                                                                      << testInfo->name()).build();
                }
            }

            util::ILogger &ClientTestSupport::getLogger() {
                return *logger;
            }

            const std::string &ClientTestSupport::getTestName() const {
                return testName;
            }
        }
    }
}
