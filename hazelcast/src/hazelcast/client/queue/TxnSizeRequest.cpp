//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/TxnSizeRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            TxnSizeRequest::TxnSizeRequest(const std::string &name)
            :name(name) {

            }

            int TxnSizeRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int TxnSizeRequest::getClassId() const {
                return QueuePortableHook::TXN_SIZE;
            }

            void TxnSizeRequest::write(serialization::PortableWriter &writer) const {
                BaseTxnRequest::write(writer);
                writer.writeUTF("n", name);
            };


        }
    }
}