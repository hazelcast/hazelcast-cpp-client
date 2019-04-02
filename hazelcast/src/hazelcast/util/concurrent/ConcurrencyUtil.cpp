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

#include "hazelcast/util/concurrent/ConcurrencyUtil.h"

namespace hazelcast {
    namespace util {
        namespace concurrent {
            const boost::shared_ptr<util::Executor> ConcurrencyUtil::callerRunsExecutor(
                    new ConcurrencyUtil::CallerThreadExecutor);

            const boost::shared_ptr<util::Executor> &ConcurrencyUtil::CALLER_RUNS() {
                return callerRunsExecutor;
            }

            void ConcurrencyUtil::CallerThreadExecutor::execute(const boost::shared_ptr<Runnable> &command) {
                command->run();
            }
        }
    }
}
