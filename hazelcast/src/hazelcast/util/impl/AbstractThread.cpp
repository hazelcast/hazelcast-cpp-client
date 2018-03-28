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

#include "hazelcast/util/impl/AbstractThread.h"

namespace hazelcast {
    namespace util {
        namespace impl {

            AbstractThread::AbstractThread(const boost::shared_ptr<Runnable> &runnable) : target(runnable) {
                this->target = runnable;
            }

            AbstractThread::~AbstractThread() {
            }

            const std::string AbstractThread::getName() const {
                if (target.get() == NULL) {
                    return "";
                }

                return target->getName();
            }

            void AbstractThread::start() {
                if (!started.compareAndSet(false, true)) {
                    return;
                }
                if (target.get() == NULL) {
                    return;
                }

                startInternal(target.get());
            }
        }
    }
}
