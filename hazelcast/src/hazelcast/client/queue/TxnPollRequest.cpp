//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/TxnPollRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            TxnPollRequest::TxnPollRequest(const std::string& name, long timeout)
            :name(name)
            , timeout(timeout) {

            }

            int TxnPollRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int TxnPollRequest::getClassId() const {
                return QueuePortableHook::TXN_POLL;
            }

            void TxnPollRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", timeout);
            };


        }
    }
}