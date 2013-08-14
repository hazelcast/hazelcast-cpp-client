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
            class QueryRequest : public Portable {
            public:
                QueryRequest(const std::string& name, const std::string& iterationType, const std::string& sql)
                :name(name)
                , iterationType(iterationType)
                , sql(sql) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::SQL_QUERY;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeUTF("t", iterationType);
                    writer.writeUTF("sql", sql);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    iterationType = reader.readUTF("t");
                    sql = reader.readUTF("sql");

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
