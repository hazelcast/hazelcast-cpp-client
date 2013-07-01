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
            class QueryDataResultStream {
            public:
                QueryDataResultStream(const std::string& name, const std::string& iterationType, const std::string& sql)
                :iterationType(iterationType)
                , isSet(true)
                , s(0) {

                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
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

                int size() const {
                    return s;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << isSet;
                    writer << iterationType;
                    writer << s;
                    for (int i = 0; i < s; i++) {
                        writer << q;
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader >> isSet;
                    reader >> iterationType;
                    reader >> s;
                    q.resize(s);
                    for (int i = 0; i < s; i++) {
                        reader >> q;
                    }

                };
            private:
                std::string iterationType;
                std::vector<QueryResultEntry> q;
                int s;
                bool isSet;
            };

        }
    }
}

#endif //HAZELCAST_QUERY_DATA_RESULT_STREAM
