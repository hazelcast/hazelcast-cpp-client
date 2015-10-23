/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 28/01/14.
//


#ifndef HAZELCAST_MemberAttributeEvent
#define HAZELCAST_MemberAttributeEvent


#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/util/IOUtil.h"
#include <boost/smart_ptr/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {

        class Cluster;

        class Member;

        /**
         * Membership event fired when a new member is added
         * to the cluster and/or when a member leaves the cluster.
         *
         * @see MembershipListener
         */
        class HAZELCAST_API MemberAttributeEvent : public MembershipEvent {
        public:
            /**
            * PUT even type representing an addition of an attribute
            * REMOVE event type representing a deletion of an attribute
            */
            enum MemberAttributeOperationType {
                PUT = 1,
                REMOVE = 2
            };

            /**
            * InternalAPI. constructor
            */
            MemberAttributeEvent(Cluster &cluster, Member &member, MemberAttributeOperationType operationType, const std::string &key, const std::string &value, util::IOUtil::PRIMITIVE_ID primitive_id);

            /**
             *
             *  enum MemberAttributeOperationType {
             *   PUT = 2,
             *   REMOVE = 3
             *   };
             * @return map operation type put or removed
             */
            MemberAttributeOperationType getOperationType() const;

            /**
             * @return key of changed attribute
             */
            const std::string &getKey() const;

            /**
             * Returns null if
             *  => given type T is not compatible with available type, or
             *  => MemberAttributeOperationType is remove(REMOVE).
             * @return value of changed attribute.
             */
            template<typename T>
            boost::shared_ptr<T> getValue() const {
                T *tag = NULL;
                boost::shared_ptr<T> v(getValueResolved(tag));
                return v;
            }

        private:
            MemberAttributeOperationType operationType;
            std::string key;
            std::string value;
            util::IOUtil::PRIMITIVE_ID primitive_id;

            int *getValueResolved(int *tag) const;

            float *getValueResolved(float *tag) const;

            short *getValueResolved(short *tag) const;

            long *getValueResolved(long *tag) const;

            byte *getValueResolved(byte *tag) const;

            bool *getValueResolved(bool *tag) const;

            std::string *getValueResolved(std::string *tag) const;

            double *getValueResolved(double *tag) const;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_MemberAttributeEvent

