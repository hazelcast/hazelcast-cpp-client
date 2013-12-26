//
// Created by sancar koyunlu on 23/12/13.
//

#ifndef HAZELCAST_DataAdapter
#define HAZELCAST_DataAdapter

#include "hazelcast/util/CircularBuffer.h"
#include "hazelcast/client/serialization/Data.h"
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationContext;

            class DataAdapter {
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

                DataAdapter(const Data& data);

                Data &getData();

                bool readFrom(util::CircularBuffer &buffer);

                bool writeTo(util::CircularBuffer& destination);

            private:
                int status;
                int factoryId;
                int classId;
                int version;
                int classDefSize;
                bool skipClassDef;
                int bytesRead;
                Data data;
                SerializationContext* context;

                void setStatus(StatusBit bit);

                bool isStatusSet(StatusBit bit) const;
            };
        }
    }
}

#endif //HAZELCAST_DataAdapter
