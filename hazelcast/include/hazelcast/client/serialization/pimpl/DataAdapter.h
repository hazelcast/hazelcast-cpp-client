//
// Created by sancar koyunlu on 23/12/13.
//

#ifndef HAZELCAST_DataAdapter
#define HAZELCAST_DataAdapter

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class SerializationContext;

                class HAZELCAST_API DataAdapter {
                    enum StatusBit {
                        stType,
                        stClassId,
                        stFactoryId,
                        stVersion,
                        stClassDefSize,
                        stClassDef,
                        stSize,
                        stValue,
                        stHash,
                        stAll
                    };
                public:
                    DataAdapter();

                    DataAdapter(const Data &data);

                    Data &getData();

                    bool readFrom(util::ByteBuffer &buffer);

                    bool writeTo(util::ByteBuffer &destination);

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
                    SerializationContext *context;

                    void setStatus(StatusBit bit);

                    bool isStatusSet(StatusBit bit) const;
                };
            }
        }
    }
}

#endif //HAZELCAST_DataAdapter
