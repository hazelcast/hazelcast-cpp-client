//
// Created by sancar koyunlu on 6/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUERY_REQUEST
#define HAZELCAST_QUERY_REQUEST

#include "../serialization/SerializationConstants.h"
#include "PortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class QueryRequest {
            public:
                QueryRequest(const std::string& name, const std::string& iterationType, const std::string& sql)
                :name(name)
                , iterationType(iterationType)
                , sql(sql) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::SQL_QUERY;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["t"] << iterationType;
                    writer["sql"] << sql;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["t"] >> iterationType;
                    reader["sql"] >> sql;

                };
            private:
                std::string name;
                std::string iterationType;
                std::string sql;
            };
        }
    }
}

#endif //HAZELCAST_QUERY_REQUEST
