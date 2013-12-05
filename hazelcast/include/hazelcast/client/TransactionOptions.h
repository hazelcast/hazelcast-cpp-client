//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_TransactionOptions
#define HAZELCAST_TransactionOptions

#include "hazelcast/client/DataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;
        };

        class HAZELCAST_API TransactionType {
        public:
            enum Type {
                TWO_PHASE = 1,
                LOCAL = 2
            } value;

            TransactionType(Type value);

            operator int() const;

            void operator = (int i);

        };

        class HAZELCAST_API TransactionOptions : public DataSerializable {
        public:
            TransactionOptions();

            TransactionType getTransactionType() const;

            TransactionOptions& setTransactionType(TransactionType transactionType);

            long getTimeoutMillis() const;

            TransactionOptions& setTimeout(long timeoutInMillis);

            int getDurability() const;

            TransactionOptions& setDurability(int durability);

            void writeData(serialization::ObjectDataOutput& out) const;

            void readData(serialization::ObjectDataInput& in);

        private:
            long timeoutMillis;

            int durability;

            TransactionType transactionType;

        };
    }
}

#endif //HAZELCAST_TransactionOptions
