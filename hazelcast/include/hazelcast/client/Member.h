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
// Created by sancar koyunlu on 5/29/13.



#ifndef HAZELCAST_MEMBER
#define HAZELCAST_MEMBER

#include <map>
#include "hazelcast/client/Address.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class ClusterListenerThread;
        }

        /**
         * Cluster member class. The default implementation
         *
         * @see Cluster
         * @see MembershipListener
         */
        class HAZELCAST_API Member {
        public:
            friend class connection::ClusterListenerThread;

            /**
            * PUT even type representing an addition of an attribute
            * REMOVE event type representing a deletion of an attribute
            */
            enum MemberAttributeOperationType {
                PUT = 1,
                REMOVE = 2
            };

            Member();

            Member(const Address &address, const std::string &uuid, bool lite,
                   const std::map<std::string, std::string> &attr);

            Member(const Address &memberAddress);

            /**
             * comparison operation
             */
            bool operator==(const Member &) const;

            /**
             *
             * Lite member is does not hold data.
             * @return true if member is lite.
             */
            bool isLiteMember() const;

            /**
             * Returns the socket address of this member.
             *
             * @return socket address of this member
             */
            const Address &getAddress() const;

            /**
             * Returns UUID of this member.
             *
             * @return UUID of this member.
             */
            const std::string &getUuid() const;

            const std::map<std::string, std::string> &getAttributes() const;

            /**
             * Returns the value of the specified key for this member or
             * default constructed value if value is undefined.
             *
             * @tparam AttributeType type template for attribute type
             * @param key The key to lookup.
             * @return The value for this members key.
             */
            const std::string *getAttribute(const std::string &key) const;

            /**
             * check if an attribute is defined for given key.
             *
             * @tparam key for the attribute
             * @return true if attribute is defined.
             */
            bool lookupAttribute(const std::string &key) const;

        private:
            void setAttribute(const std::string &key, const std::string &value);

            bool removeAttribute(const std::string &key);

            Address address;
            std::string uuid;
            bool liteMember;
            std::map<std::string, std::string> attributes;
        };

        std::ostream HAZELCAST_API &operator<<(std::ostream &out, const Member &member);
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#endif //HAZELCAST_MEMBER

