//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/topic/PortableMessage.h"
#include "hazelcast/client/topic/TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace topic {

            PortableMessage::PortableMessage() {

            };

            PortableMessage::PortableMessage(const serialization::Data& message, long publishTime, const std::string& uuid)
            : uuid(uuid)
            , message(message)
            , publishTime(publishTime) {

            };

            const serialization::Data& PortableMessage::getMessage() const {
                return message;
            };

            std::string PortableMessage::getUuid() const {
                return uuid;
            };

            long PortableMessage::getPublishTime() const {
                return publishTime;
            };

            int PortableMessage::getFactoryId() const {
                return TopicPortableHook::F_ID;
            };

            int PortableMessage::getClassId() const {
                return TopicPortableHook::PORTABLE_MESSAGE;
            };

            void PortableMessage::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("pt", publishTime);
                writer.writeUTF("u", uuid);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                message.writeData(out);
            };

            void PortableMessage::readPortable(serialization::PortableReader& reader) {
                publishTime = reader.readLong("pt");
                uuid = reader.readUTF("u");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                message.readData(in);
            };
        }
    }
}

