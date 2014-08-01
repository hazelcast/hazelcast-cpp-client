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

                    boost::shared_ptr<ClassDefinition> getClassDefinition(const Portable& p);

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
#endif /* HAZELCAST_PORTABLE_SERIALIZER */

