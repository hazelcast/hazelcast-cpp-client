//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/TxnOfferRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            TxnOfferRequest::TxnOfferRequest(const std::string& name, long timeoutInMillis, const serialization::pimpl::Data& data)
            : name(name)
            , data(data)
            , timeoutInMillis(timeoutInMillis) {

            }

            int TxnOfferRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int TxnOfferRequest::getClassId() const {
                return QueuePortableHook::TXN_OFFER;
            }

            void TxnOfferRequest::write(serialization::PortableWriter& writer) const {
                BaseTxnRequest::write(writer);
                writer.writeUTF("n", name);
                writer.writeLong("t", timeoutInMillis);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeData(&data);
            }

        }
    }
}
