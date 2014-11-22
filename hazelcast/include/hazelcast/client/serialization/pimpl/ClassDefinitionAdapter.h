//
// Created by sancar koyunlu on 17/11/14.
//


#ifndef HAZELCAST_ClassDefinitionAdapter
#define HAZELCAST_ClassDefinitionAdapter

#include "hazelcast/util/HazelcastDll.h"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace hazelcast {
    namespace util{
        class ByteBuffer;
    }

    namespace client {

        namespace serialization {

            class ClassDefinition;

            namespace pimpl {
                class Data;

                class PortableContext;

                class ClassDefinitionAdapter {
                public:
                    ClassDefinitionAdapter(pimpl::Data& data, pimpl::PortableContext& portableContext);

                    bool write(util::ByteBuffer& destination);

                    bool read(util::ByteBuffer& source);

                private:
                    static const int CLASS_DEF_HEADER_SIZE;
                    static const int ST_PREPARED;
                    static const int ST_HEADER;
                    static const int ST_DATA;
                    static const int ST_SKIP_DATA;

                    // common fields
                    pimpl::Data& data;
                    pimpl::PortableContext& context;
                    byte status;

                    // write fields
                    std::vector<boost::shared_ptr<ClassDefinition> > classDefinitions;
                    size_t classDefCount;
                    size_t classDefIndex;
                    size_t bytesWrittenPerClassDefinition;

                    // read fields
                    size_t classDefSize;
                    int factoryId;
                    std::auto_ptr<std::vector<byte> > metadata;
                    std::auto_ptr<std::vector<byte> > buffer;
                    size_t bytesReadPerClassDefinition;


                    //write methods
                    bool writeData(ClassDefinition &cd, util::ByteBuffer& destination);

                    bool writeAll(util::ByteBuffer& destination);

                    bool writeHeader(ClassDefinition &cd, util::ByteBuffer& destination);

                    bool flushBuffer(std::vector<byte> const& binary, util::ByteBuffer& destination);

                    //read methods
                    bool readData(hazelcast::util::ByteBuffer& destination);

                    bool readAll(hazelcast::util::ByteBuffer& destination);

                    bool readHeader(hazelcast::util::ByteBuffer& destination);

                    //read write common methods
                    bool isStatusSet(int bit) const;

                    void setStatus(int bit);

                    void clearStatus(int bit) ;
                };
            }
        }
    }
}


#endif //HAZELCAST_ClassDefinitionAdapter
