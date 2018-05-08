/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/2/13.



#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/exception/IllegalStateException.h"

#define SECONDS_IN_A_MINUTE     60

namespace hazelcast {
    namespace client {
        TransactionOptions::TransactionOptions()
        : timeoutSeconds(2 * SECONDS_IN_A_MINUTE)//2 minutes
        , durability(1)
        , transactionType(TransactionType::TWO_PHASE) {

        }

        TransactionType TransactionOptions::getTransactionType() const {
            return transactionType;
        }

        TransactionOptions &TransactionOptions::setTransactionType(TransactionType transactionType) {
            this->transactionType = transactionType;
            return *this;
        }

        int TransactionOptions::getTimeout() const {
            return timeoutSeconds;
        }

        TransactionOptions &TransactionOptions::setTimeout(int timeoutInSeconds) {
            if (timeoutInSeconds <= 0) {
                throw exception::IllegalStateException("TransactionOptions::setTimeout", "Timeout must be positive!");
            }
            this->timeoutSeconds = timeoutInSeconds;
            return *this;
        }

        int TransactionOptions::getDurability() const {
            return durability;
        }

        TransactionOptions &TransactionOptions::setDurability(int durability) {
            if (durability < 0) {
                throw exception::IllegalStateException("TransactionOptions::setDurability", "Durability cannot be negative!");
            }
            this->durability = durability;
            return *this;
        }

        int TransactionOptions::getTimeoutMillis() const {
            return timeoutSeconds * 1000;
        }

        TransactionType::TransactionType(Type value):value(value) {
        }

        TransactionType::operator int() const {
            return value;
        }

        void TransactionType::operator = (int i) {
            if (i == TWO_PHASE) {
                value = TWO_PHASE;
            } else {
                value = LOCAL;
            }
        }
    }
}
