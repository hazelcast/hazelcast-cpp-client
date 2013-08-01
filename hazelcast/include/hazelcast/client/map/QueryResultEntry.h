//
// Created by sancar koyunlu on 6/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUERY_RESULT_ENTRY
#define HAZELCAST_QUERY_RESULT_ENTRY


#include "../serialization/Data.h"


namespace hazelcast {
    namespace client {
        namespace map {
            class QueryResultEntry : public DataSerializable {
            public:
                QueryResultEntry() {

                };

                int getFactoryId() const {
                    return DataSerializableHook::F_ID;
                }

                int getClassId() const {
                    return DataSerializableHook::QUERY_RESULT_ENTRY;
                }

                void writeData(serialization::ObjectDataOutput& writer) const {
                    writer.writeBoolean(true);
                    keyIndex.writeData(writer);
                    writer.writeBoolean(true);
                    key.writeData(writer);
                    writer.writeBoolean(true);
                    value.writeData(writer);
                };

                void readData(serialization::ObjectDataInput& reader) {
                    bool isNotNull = reader.readBoolean();
                    if (isNotNull)
                        keyIndex.readData(reader);
                    isNotNull = reader.readBoolean();
                    if (isNotNull)
                        key.readData(reader);
                    isNotNull = reader.readBoolean();
                    if (isNotNull)
                        value.readData(reader);
                };
                serialization::Data keyIndex;
                serialization::Data key;
                serialization::Data value;
            };
        }
    }
}

#endif //__QueryResultEntry_H_
