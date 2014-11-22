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

                    Data();

                    Data(const Data&);

                    Data& operator=(const Data&);

                    int bufferSize() const;

                    size_t headerSize() const;

                    int getPartitionHash() const;

                    void setPartitionHash(int);

                    int getType() const;

                    void setType(int type);

                    void setBuffer(std::auto_ptr<std::vector<byte> > buffer);

                    void setHeader(std::auto_ptr<std::vector<byte> > header);

                    bool hasPartitionHash()  const;

                    bool isPortable() const;

                    bool hasClassDefinition() const;

                    size_t getClassDefinitionCount() const;

                    std::vector<boost::shared_ptr<ClassDefinition> > getClassDefinitions(PortableContext& context) const;

                    mutable std::auto_ptr<std::vector<byte> > data;

                    mutable std::auto_ptr<std::vector<byte> > header;
                    static int HEADER_ENTRY_LENGTH;

                    static int HEADER_FACTORY_OFFSET;
                    static int HEADER_CLASS_OFFSET;
                    static int HEADER_VERSION_OFFSET;
                private:

                    mutable int partitionHash;
                    int type;

                    boost::shared_ptr<ClassDefinition> readClassDefinition(PortableContext& context, int start) const;

                    int readIntHeader(int offset) const;

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

