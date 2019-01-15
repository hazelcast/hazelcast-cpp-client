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

#ifndef HAZELCAST_CLIENT_MULTIEXECUTIONCALLBACK_H_
#define HAZELCAST_CLIENT_MULTIEXECUTIONCALLBACK_H_

#include <map>
#include <boost/shared_ptr.hpp>

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
         * the {@link #onComplete(std::map)} method is called with a map of all results.
         *
         * @see IExecutorService
         * @see ExecutionCallback
         */
        template<typename V>
        class MultiExecutionCallback {
        public:
            virtual ~MultiExecutionCallback() {}

            /**
             * Called when an execution is completed on a member.
             *
             * @param member member that the task is submitted to.
             * @param value result of the execution
             */
            virtual void onResponse(const Member &member, const boost::shared_ptr<V> &response) = 0;

            /**
             * Called when an execution is completed with an exception on a member.
             *
             * @param member member that the task is submitted to.
             * @param exception result of the execution
             */
            virtual void onFailure(const Member &member, const boost::shared_ptr<exception::IException> &exception) = 0;

            /**
             * Called after all executions are completed.
             *
             * @param values map of Member-Response pairs where no exception occured.
             * @param exceptions The exceptions produced by failing members.
             */
            virtual void onComplete(const std::map<Member, boost::shared_ptr<V> > &values,
                                    const std::map<Member, boost::shared_ptr<exception::IException> > &exceptions) = 0;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_MULTIEXECUTIONCALLBACK_H_

