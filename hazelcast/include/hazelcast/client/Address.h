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
#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

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
        class HAZELCAST_API Address : public serialization::IdentifiedDataSerializable {
        public:
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
             * @return host address as string
             */
            const std::string& getHost() const;

            /***** serialization::IdentifiedDataSerializable interface implementation starts here *********************/
            virtual int getFactoryId() const;

            virtual int getClassId() const;

            virtual void writeData(serialization::ObjectDataOutput &writer) const;

            virtual void readData(serialization::ObjectDataInput &reader);

            /***** serialization::IdentifiedDataSerializable interface implementation end here ************************/
        private:
            std::string host;
            int port;
            byte type;

            static const byte IPV4;
            static const byte IPV6;
        };

        /**
         * Address comparator functor
         */
        struct HAZELCAST_API addressComparator {
            /**
             * Address comparator functor
             * @param lhs
             * @param rhs
             */
            bool operator ()(const Address &lhs, const Address &rhs) const;
        };

		std::ostream HAZELCAST_API &operator << (std::ostream &stream, const Address &address);

    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_ADDRESS */
