//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PublishRequest.h"
#include "TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            PublishRequest::PublishRequest(const std::string& instanceName, const serialization::Data& message)
            : instanceName(instanceName)
            , message(message) {

            };

            int PublishRequest::getFactoryId() const {
                return TopicPortableHook::F_ID;
            };

            int PublishRequest::getClassId() const {
                return TopicPortableHook::PUBLISH;
            };


            void PublishRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", instanceName);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                message.writeData(out);
            };


            void PublishRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                message.readData(in);
            };
        }
    }
}

