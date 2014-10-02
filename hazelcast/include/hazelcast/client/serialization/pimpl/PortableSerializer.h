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

                    void write(DataOutput& dataOutput, const Portable& p);

                    void read(DataInput& dataInput, Portable& portable, int factoryId, int classId, int dataVersion);

                private:
                    PortableContext& context;

                    int findPortableVersion(int factoryId, int classId, const Portable& portable);

                    PortableReader createReader(DataInput& input, int factoryId, int classId, int version, int portableVersion);
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_PORTABLE_SERIALIZER */

