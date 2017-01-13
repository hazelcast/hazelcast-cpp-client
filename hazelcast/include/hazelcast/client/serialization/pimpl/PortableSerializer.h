/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_SERIALIZER
#define HAZELCAST_PORTABLE_SERIALIZER

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <map>
#include <memory>
#include <boost/shared_ptr.hpp>


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class Portable;

            class PortableReader;

            namespace pimpl {
                class DataOutput;

                class DataInput;

                class PortableContext;

                class HAZELCAST_API PortableSerializer {
                public:

                    PortableSerializer(PortableContext& portableContext);

                    void write(DataOutput& dataOutput, const Portable& p) const;

                    void read(DataInput &in, Portable &p, int factoryId, int classId) const;

                private:
                    PortableContext& context;

                    int findPortableVersion(int factoryId, int classId, const Portable& portable) const;

                    PortableReader createReader(DataInput& input, int factoryId, int classId, int version, int portableVersion) const;
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_PORTABLE_SERIALIZER */

