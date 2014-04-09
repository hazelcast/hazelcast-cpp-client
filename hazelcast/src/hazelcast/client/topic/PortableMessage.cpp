//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/topic/PortableMessage.h"
#include "hazelcast/client/topic/TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace topic {

            const serialization::pimpl::Data& PortableMessage::getMessage() const {
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

            void PortableMessage::readPortable(serialization::PortableReader& reader) {
                publishTime = reader.readLong("pt");
                uuid = reader.readUTF("u");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                message.readData(in);
            };
        }
    }
}


