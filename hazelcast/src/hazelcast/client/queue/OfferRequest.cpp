//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/OfferRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            OfferRequest::OfferRequest(const std::string &name, serialization::pimpl::Data &data, long timeout)
            :name(name)
            , timeoutInMillis(timeout)
            , data(data) {

            };

            int OfferRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int OfferRequest::getClassId() const {
                return queue::QueuePortableHook::OFFER;
            }


            void OfferRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", timeoutInMillis);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                data.writeData(out);
            };
        }
    }
}

