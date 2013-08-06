//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "TransactionOptions.h"
#include "IllegalStateException.h"

namespace hazelcast {
    namespace client {
            TransactionOptions::TransactionOptions()
            : timeoutMillis(2 * 1000 * 60)//2 minutes
            , durability(1)
            , transactionType(TransactionType::TWO_PHASE) {

            };

            TransactionType TransactionOptions::getTransactionType() const {
                return transactionType;
            };

            TransactionOptions& TransactionOptions::setTransactionType(TransactionType transactionType) {
                this->transactionType = transactionType;
                return *this;
            };

            long TransactionOptions::getTimeoutMillis() const {
                return timeoutMillis;
            };

            TransactionOptions& TransactionOptions::setTimeout(long timeoutInMillis) {
                if (timeoutInMillis <= 0) {
                    throw exception::IllegalStateException("TransactionOptions::setTimeout", "Timeout must be positive!");
                }
                this->timeoutMillis = timeoutInMillis;
                return *this;
            };

            int TransactionOptions::getDurability() const {
                return durability;
            };

            TransactionOptions& TransactionOptions::setDurability(int durability) {
                if (durability < 0) {
                    throw exception::IllegalStateException("TransactionOptions::setDurability", "Durability cannot be negative!");
                }
                this->durability = durability;
                return *this;
            };

            void TransactionOptions::writeData(serialization::ObjectDataOutput& out) const {
                out.writeLong(timeoutMillis);
                out.writeInt(durability);
                out.writeInt(transactionType);
            };

            void TransactionOptions::readData(serialization::ObjectDataInput& in) {
                timeoutMillis = in.readLong();
                durability = in.readInt();
                transactionType = in.readInt();
            };

            TransactionType::TransactionType(Type value):value(value) {

            };

            TransactionType::operator int() const{
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