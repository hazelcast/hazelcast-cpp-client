//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/exception/IllegalStateException.h"

namespace hazelcast {
    namespace client {
        TransactionOptions::TransactionOptions()
        : timeoutSeconds(2 * 60)//2 minutes
        , durability(1)
        , transactionType(TransactionType::TWO_PHASE) {

        };

        TransactionType TransactionOptions::getTransactionType() const {
            return transactionType;
        };

        TransactionOptions &TransactionOptions::setTransactionType(TransactionType transactionType) {
            this->transactionType = transactionType;
            return *this;
        };

        int TransactionOptions::getTimeout() const {
            return timeoutSeconds;
        };

        TransactionOptions &TransactionOptions::setTimeout(int timeoutInSeconds) {
            if (timeoutInSeconds <= 0) {
                throw exception::IllegalStateException("TransactionOptions::setTimeout", "Timeout must be positive!");
            }
            this->timeoutSeconds = timeoutInSeconds;
            return *this;
        };

        int TransactionOptions::getDurability() const {
            return durability;
        };

        TransactionOptions &TransactionOptions::setDurability(int durability) {
            if (durability < 0) {
                throw exception::IllegalStateException("TransactionOptions::setDurability", "Durability cannot be negative!");
            }
            this->durability = durability;
            return *this;
        };

        void TransactionOptions::writeData(serialization::ObjectDataOutput &out) const {
            out.writeLong(1000L * timeoutSeconds);
            out.writeInt(durability);
            out.writeInt(transactionType);
        };

        void TransactionOptions::readData(serialization::ObjectDataInput &in) {
            timeoutSeconds = (int)in.readLong()/1000;
            durability = in.readInt();
            transactionType = in.readInt();
        };

        TransactionType::TransactionType(Type value):value(value) {

        };

        TransactionType::operator int() const {
            return value;
        };

        void TransactionType::operator = (int i) {
            if (i == TWO_PHASE) {
                value = TWO_PHASE;
            } else {
                value = LOCAL;
            }
        };

    }
}
