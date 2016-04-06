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
#ifndef HAZELCAST_CLIENT_QUERY_INSTANCEOFPREDICATE_H_
#define HAZELCAST_CLIENT_QUERY_INSTANCEOFPREDICATE_H_

#include <memory>
#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace query {
            class HAZELCAST_API InstanceOfPredicate : public serialization::IdentifiedDataSerializable {
            public:
                /**
                 * @param javaClassName The name of the java class as identified by Class.getName() in java.
                 */
                InstanceOfPredicate(const char *javaClassName);

                /**
                 * @return factory id
                 */
                int getFactoryId() const;

                /**
                 * @return class id
                 */
                int getClassId() const;

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                void writeData(serialization::ObjectDataOutput &out) const;

                /**
                 *Defines how this class will be read.
                 * @param reader ObjectDataInput
                 */
                void readData(serialization::ObjectDataInput &in);
            private:
                std::string className;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_QUERY_INSTANCEOFPREDICATE_H_ */
