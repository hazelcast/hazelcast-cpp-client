//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "OfferRequest.h"
#include "QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            OfferRequest::OfferRequest(const std::string& name, serialization::Data& data, long timeout)
            :name(name)
            , timeoutInMillis(timeout)
            , data(data) {

            };

            OfferRequest::OfferRequest(const std::string& name, serialization::Data& data)
            :name(name)
            , timeoutInMillis(0)
            , data(data) {

            };

            int OfferRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int OfferRequest::getClassId() const {
                return queue::QueuePortableHook::OFFER;
            }


            void OfferRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", timeoutInMillis);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                data.writeData(out);
            };


            void OfferRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                timeoutInMillis = reader.readLong("t");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                data.readData(in);
            };
        }
    }
}
