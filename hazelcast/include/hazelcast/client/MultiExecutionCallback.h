/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <unordered_map>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/Member.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * MultiExecutionCallback provides notification for when an execution is completed on each member
         * that a task is submitted to. After all executions are completed on all submitted members,
         * the {@link #onComplete(std::unordered_map)} method is called with a map of all results.
         *
         * @see IExecutorService
         * @see ExecutionCallback
         */
        template<typename V>
        class MultiExecutionCallback {
        public:
            virtual ~MultiExecutionCallback() = default;

            /**
             * Called when an execution is completed on a member.
             *
             * @param member member that the task is submitted to.
             * @param value result of the execution
             */
            virtual void onResponse(const Member &member, const boost::optional<V> &response) = 0;

            /**
             * Called when an execution is completed with an exception on a member.
             *
             * @param member member that the task is submitted to.
             * @param exception result of the execution
             */
            virtual void onFailure(const Member &member, const std::exception_ptr exception) = 0;

            /**
             * Called after all executions are completed.
             *
             * @param values map of Member-Response pairs where no exception occured.
             * @param exceptions The exceptions produced by failing members.
             */
            virtual void onComplete(const std::unordered_map<Member, boost::optional<V> > &values,
                                    const std::unordered_map<Member, std::exception_ptr> &exceptions) = 0;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



