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
#include <cassert>

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/connection/AuthenticationFuture.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            AuthenticationFuture::AuthenticationFuture() : countDownLatch(new util::CountDownLatch(1)) {
            }

            void AuthenticationFuture::onSuccess(const std::shared_ptr<Connection> &connection) {
                this->connection = connection;
                countDownLatch->countDown();
            }

            void AuthenticationFuture::onFailure(const std::shared_ptr<exception::IException> &throwable) {
                this->throwable = throwable;
                countDownLatch->countDown();
            }

            std::shared_ptr<Connection> AuthenticationFuture::get() {
                countDownLatch->await();
                if (connection.get() != NULL) {
                    return connection;
                }
                assert(throwable.get() != NULL);
                throw exception::ExecutionException("AuthenticationFuture::get", "Could not be authenticated.",
                                                    throwable);
            }

        }
    }
}
