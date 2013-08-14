//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PortableMessage
#define HAZELCAST_PortableMessage

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "TopicPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class PortableMessage : public Portable{
            public:
                PortableMessage(const serialization::Data& message, long publishTime, const std::string& uuid)
                : uuid(uuid)
                , message(message)
                , publishTime(publishTime) {

                };

                const serialization::Data& getMessage() const {
                    return message;
                };

                std::string getUuid() const {
                    return uuid;
                };

                long getPublishTime() const {
                    return publishTime;
                };

                int getFactoryId() const {
                    return TopicPortableHook::F_ID;
                };

                int getClassId() const {
                    return TopicPortableHook::PORTABLE_MESSAGE;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeLong("pt", publishTime);
                    writer.writeUTF("u", uuid);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    message.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    publishTime = reader.readLong("pt");
                    uuid = reader.readUTF("u");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    message.readData(in);
                };
            private:
                serialization::Data message;
                std::string uuid;
                long publishTime;
            };
        }
    }
}


#endif //HAZELCAST_PortableMessage
