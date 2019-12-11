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
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

#include <hazelcast/util/ExceptionUtil.h>

#include "hazelcast/client/exception/ProtocolExceptions.h"

namespace hazelcast {
    namespace util {
        const boost::shared_ptr<ExceptionUtil::RuntimeExceptionFactory> ExceptionUtil::hazelcastExceptionFactory(
                new HazelcastExceptionFactory());

        void ExceptionUtil::rethrow(const client::exception::IException &e) {
            return rethrow(e, HAZELCAST_EXCEPTION_FACTORY());
        }

        void ExceptionUtil::rethrow(const client::exception::IException &e,
                                    const boost::shared_ptr<ExceptionUtil::RuntimeExceptionFactory> &runtimeExceptionFactory) {
            if (e.isRuntimeException()) {
                e.raise();
            }

            int32_t errorCode = e.getErrorCode();
            if (errorCode == client::exception::ExecutionException::ERROR_CODE) {
                boost::shared_ptr<client::exception::IException> cause = e.getCause();
                if (cause.get() != NULL) {
                    return rethrow(*cause, runtimeExceptionFactory);
                }
            }

            runtimeExceptionFactory->rethrow(e, "");
        }

        const boost::shared_ptr<ExceptionUtil::RuntimeExceptionFactory> &ExceptionUtil::HAZELCAST_EXCEPTION_FACTORY() {
            return hazelcastExceptionFactory;
        }

        ExceptionUtil::RuntimeExceptionFactory::~RuntimeExceptionFactory() {
        }

        void ExceptionUtil::HazelcastExceptionFactory::rethrow(
                const client::exception::IException &throwable, const std::string &message) {
            throw client::exception::HazelcastException("HazelcastExceptionFactory::create", message,
                                                        boost::shared_ptr<client::exception::IException>(
                                                                throwable.clone()));
        }
    }
}

