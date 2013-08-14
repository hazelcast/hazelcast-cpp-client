//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnPollRequest.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            TxnPollRequest::TxnPollRequest(const std::string& name)
            :name(name){

            }

            int TxnPollRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int TxnPollRequest::getClassId() const {
                return QueuePortableHook::TXN_POLL;
            }


        }
    }
}