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

#include <gtest/gtest.h>

#include <hazelcast/util/RuntimeAvailableProcessors.h>

using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace test {
            class RuntimeAvailableProcessorsTest : public ::testing::Test {
            protected:
                virtual void TearDown() {
                    RuntimeAvailableProcessors::resetOverride();
                }
            };

            TEST_F(RuntimeAvailableProcessorsTest, getAvailableProcessors_withoutOverride) {
                int availableProcessors = RuntimeAvailableProcessors::getNumberOfProcessors();
                ASSERT_EQ(availableProcessors, RuntimeAvailableProcessors::get());
            }

            TEST_F(RuntimeAvailableProcessorsTest, getAvailableProcessors_withOverride) {
                int customAvailableProcessors = 1234;
                RuntimeAvailableProcessors::override(customAvailableProcessors);
                try {
                    ASSERT_EQ(customAvailableProcessors, RuntimeAvailableProcessors::get());
                } catch (...) {
                        RuntimeAvailableProcessors::resetOverride();
                }
            }
        }
    }
}
