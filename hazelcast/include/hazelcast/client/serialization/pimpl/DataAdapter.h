//
// Created by sancar koyunlu on 23/12/13.
//

#ifndef HAZELCAST_DataAdapter
#define HAZELCAST_DataAdapter

#include "hazelcast/client/serialization/pimpl/Data.h"
#include <memory>

namespace hazelcast {
    namespace util{
        class ByteBuffer;
    }
    namespace client {
        namespace serialization {
            namespace pimpl {
                class PortableContext;

                class DataAdapter {
                public:
                    DataAdapter(PortableContext& context);

                    DataAdapter(PortableContext& context, const Data& data);

                    virtual ~DataAdapter();

                    const Data& getData() const;

                    void setData(Data& data);

                    virtual bool readFrom(util::ByteBuffer& buffer);

                    virtual bool writeTo(util::ByteBuffer& destination);

                protected:
                    void setStatus(int statusBit);

                    bool isStatusSet(int statusBit) const;

                    static const int ST_TYPE = 1;
                    static const int ST_CLASS_ID = 2;
                    static const int ST_FACTORY_ID = 3;
                    static const int ST_VERSION = 4;
                    static const int ST_CLASS_DEF_SIZE = 5;
                    static const int ST_CLASS_DEF = 6;
                    static const int ST_SIZE = 7;
                    static const int ST_VALUE = 8;
                    static const int ST_HASH = 9;
                    static const int ST_ALL = 10;
                private:
                    int status;
                    int factoryId;
                    int classId;
                    int version;
                    size_t classDefSize;
                    bool skipClassDef;
                    size_t bytesRead;
                    size_t bytesWritten;
                    Data data;
                    PortableContext& context;
                };
            }
        }
    }
}

#endif //HAZELCAST_DataAdapter

