//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/util/HazelcastDll.h"
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                class HAZELCAST_API Data {
                public:
                    // type and partition_hash are always written with BIG_ENDIAN byte-order
                    static unsigned int TYPE_OFFSET;
                    // will use a byte to store partition_hash bit
                    static unsigned int PARTITION_HASH_BIT_OFFSET;
                    static unsigned int DATA_OFFSET;

                    Data();

                    Data(std::auto_ptr<std::vector<byte> > buffer);

                    Data(const Data&);

                    Data& operator=(const Data&);

                    size_t dataSize() const;

                    size_t totalSize() const;

                    int getPartitionHash() const;

                    bool hasPartitionHash() const;

                    std::vector<byte> &toByteArray() const;

                    int getType() const;

                private:
                    mutable std::auto_ptr<std::vector<byte> > data;

                    int hashCode() const;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_DATA */

