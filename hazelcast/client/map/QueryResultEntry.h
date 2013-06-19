//
// Created by sancar koyunlu on 6/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUERY_RESULT_ENTRY
#define HAZELCAST_QUERY_RESULT_ENTRY


#include "../serialization/Data.h"


namespace hazelcast {
    namespace client {
        namespace map {
            class QueryResultEntry {
            public:
                QueryResultEntry() {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
                };

                int getFactoryId() const {
                    return DataSerializableHook::F_ID;
                }

                int getClassId() const {
                    return DataSerializableHook::QUERY_RESULT_ENTRY;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << true;
                    writer << keyIndex;
                    writer << true;
                    writer << key;
                    writer << true;
                    writer << value;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    bool isNull;
                    reader >> isNull;
                    if (!isNull)
                        reader >> keyIndex;
                    reader >> isNull;
                    if (!isNull)
                        reader >> key;
                    reader >> isNull;
                    if (!isNull)
                        reader >> value;
                };
                serialization::Data keyIndex;
                serialization::Data key;
                serialization::Data value;
            };
        }
    }
}

#endif //__QueryResultEntry_H_
