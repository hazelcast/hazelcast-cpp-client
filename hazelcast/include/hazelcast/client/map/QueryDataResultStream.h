//
// Created by sancar koyunlu on 6/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUERY_DATA_RESULT_STREAM
#define HAZELCAST_QUERY_DATA_RESULT_STREAM

#include "../serialization/SerializationConstants.h"
#include "PortableHook.h"
#include "DataSerializableHook.h"
#include "Data.h"
#include "QueryResultEntry.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class QueryDataResultStream : public DataSerializable {
            public:
                QueryDataResultStream(const std::string& name, const std::string& iterationType, const std::string& sql)
                :iterationType(iterationType)
                , isSet(true) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return DataSerializableHook::QUERY_RESULT_STREAM;
                };

                const std::vector<QueryResultEntry>& getResultData() const {
                    return q;
                };

                void writePortable(serialization::BufferedDataOutput& writer) const {
                    writer.writeBoolean(isSet);
                    writer.writeUTF(iterationType);
                    writer.writeInt(q.size());
                    for (int i = 0; i < q.size(); i++) {
                        q[i].writeData(writer);
                    }
                };

                void readPortable(serialization::BufferedDataInput& reader) {
                    isSet = reader.readBoolean();
                    iterationType = reader.readUTF();
                    int size = reader.readInt();
                    q.resize(size);
                    for (int i = 0; i < size; i++) {
                        q[i].readData(reader);
                    }

                };
            private:
                std::string iterationType;
                std::vector<QueryResultEntry> q;
                bool isSet;
            };

        }
    }
}

#endif //HAZELCAST_QUERY_DATA_RESULT_STREAM
