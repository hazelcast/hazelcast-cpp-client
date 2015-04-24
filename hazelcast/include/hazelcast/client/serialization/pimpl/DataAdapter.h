//
// Created by sancar koyunlu on 23/12/13.
//

#ifndef HAZELCAST_DataAdapter
#define HAZELCAST_DataAdapter

#include "hazelcast/client/serialization/pimpl/Data.h"
#include <memory>

namespace hazelcast {
    namespace util {
        class ByteBuffer;
    }
    namespace client {
        namespace serialization {
            namespace pimpl {
                class PortableContext;

                class ClassDefinitionAdapter;

                class DataAdapter {
                public:
                    DataAdapter(PortableContext& context);

                    DataAdapter(PortableContext& context, const Data& data);

                    virtual ~DataAdapter();

                    const Data& getData() const;

                    void setData(Data& data);

                    PortableContext& getPortableContext() const;

                    virtual bool readFrom(util::ByteBuffer& buffer);

                    virtual bool writeTo(util::ByteBuffer& destination);

                protected:
                    void setStatus(int statusBit);

                    bool isStatusSet(int statusBit) const;

                    static const int ST_TYPE;
                    static const int ST_SIZE;
                    static const int ST_VALUE;
                    static const int ST_HASH;
                    static const int ST_ALL;
                private:
                    short status;
                    size_t bytesRead;
                    size_t bytesWritten;
                    Data data;
                    PortableContext& context;
                    ClassDefinitionAdapter* classDefinitionAdapter;
                };
            }
        }
    }
}

#endif //HAZELCAST_DataAdapter

