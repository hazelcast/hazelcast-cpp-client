//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnOfferRequest.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            TxnOfferRequest::TxnOfferRequest(const std::string& name, long timeoutInMillis, serialization::Data & data)
            :name(name)
            , data(data)
            , timeoutInMillis(timeoutInMillis) {

            }

            int TxnOfferRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int TxnOfferRequest::getClassId() const {
                return QueuePortableHook::TXN_OFFER;
            }

        }
    }
}