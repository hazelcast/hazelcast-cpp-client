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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/serialization.h"

#include <string>
#include <sstream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        /**
         * IP Address
         */
        class HAZELCAST_API Address {
            friend struct serialization::hz_serializer<Address>;
            friend struct std::hash<hazelcast::client::Address>;
        public:
            Address(const std::string &hostname, int port, unsigned long scopeId);

            static const int ID;

            /**
             * Constructor
             */
            Address();

            /**
             * Constructor
             */
            Address(const std::string &url, int port);

            /**
             * comparison operator
             * @param address to be compared.
             */
            bool operator == (const Address &address) const;

            /**
             * comparison operator
             * @param address to be compared.
             */
            bool operator != (const Address &address) const;

            /**
             * @return port number.
             */
            int getPort() const;

            /**
             *
             * @return true if the address is ip V4 address, false otherwise.
             */
            bool isIpV4() const;

            /**
             * @return host address as string
             */
            const std::string& getHost() const;

            unsigned long getScopeId() const;

            bool operator<(const Address &rhs) const;

            std::string toString() const;
        private:
            std::string host_;
            int port_;
            byte type_;
            unsigned long scopeId_;

            static const byte IPV4;
            static const byte IPV6;
        };

        namespace serialization {
            template<>
            struct hz_serializer<Address> : public identified_data_serializer {
                static constexpr int32_t F_ID = 0;
                static constexpr int32_t ADDRESS = 1;
                static int32_t getFactoryId();
                static int32_t getClassId();
                static void writeData(const Address &object, ObjectDataOutput &out);
                static Address readData(ObjectDataInput &in);
            };
        }

        std::ostream HAZELCAST_API &operator << (std::ostream &stream, const Address &address);
    }
}

namespace std {
    template<>
    struct HAZELCAST_API hash<hazelcast::client::Address> {
        std::size_t operator()(const hazelcast::client::Address &address) const noexcept;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 


