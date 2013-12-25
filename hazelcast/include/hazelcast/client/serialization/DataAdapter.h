//
// Created by sancar koyunlu on 23/12/13.
//

#ifndef HAZELCAST_DataAdapter
#define HAZELCAST_DataAdapter

#include "hazelcast/util/CircularBuffer.h"
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationContext;
            class Data;

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

                Data &getData();

                bool readFrom(util::CircularBuffer &buffer);

            private:
                StatusBit status;
                int factoryId;
                int classId;
                int version;
                int classDefSize;
                bool skipClassDef;
                int bytesRead;
                std::auto_ptr<Data> data;
                SerializationContext* context;

                void setStatus(StatusBit bit);

                bool isStatusSet(StatusBit bit);
            };
        }
    }
}

#endif //HAZELCAST_DataAdapter
