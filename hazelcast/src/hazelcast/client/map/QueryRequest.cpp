//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "QueryRequest.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            QueryRequest::QueryRequest(const std::string& name, const std::string& iterationType, const std::string& sql)
            :name(name)
            , iterationType(iterationType)
            , sql(sql) {

            };

            int QueryRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int QueryRequest::getClassId() const {
                return PortableHook::SQL_QUERY;
            }

            void QueryRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeUTF("t", iterationType);
                writer.writeUTF("sql", sql);
            };

            void QueryRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                iterationType = reader.readUTF("t");
                sql = reader.readUTF("sql");

            };
        }
    }
}

