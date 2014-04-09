//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_TransactionOptions
#define HAZELCAST_TransactionOptions

#include "hazelcast/client/serialization/pimpl/DataSerializable.h"

namespace hazelcast {
    namespace client {
        /**
         * Transaction type.
         */
        class HAZELCAST_API TransactionType {
        public:
            /**
             * Type enum.
             */
            enum Type {
                TWO_PHASE = 1,
                LOCAL = 2
            };
            /**
             * value.
             */
            Type value;

            /**
             * Constructor
             */
            TransactionType(Type value);

            /**
             * cast to int.
             */
            operator int() const;

            /**
             * copy constructor.
             */
            void operator = (int i);

        };

        /**
         * Contains the configuration for a Hazelcast transaction.
         */
        class HAZELCAST_API TransactionOptions : public serialization::pimpl::DataSerializable {
        public:
            /**
             * Creates a new default configured TransactionsOptions.
             *
             * It will be configured with a timeout of 2 minutes, durability of 1 and a TransactionType.TWO_PHASE.
             */
            TransactionOptions();

            /**
             *
             * @return the TransactionType.
             */
            TransactionType getTransactionType() const;

            /**
             * Sets the TransactionType.
             *
             * A local transaction is less safe than a two phase transaction; when a member fails during the commit
             * of a local transaction, it could be that some of the changes are committed, while others are not and this
             * can leave your system in an inconsistent state.
             *
             * @param transactionType the new TransactionType.
             * @return the updated TransactionOptions.
             * @see #getTransactionType()
             * @see #setDurability(int)
             */
            TransactionOptions &setTransactionType(TransactionType transactionType);

            /**
             *
             * @return the timeout in milliseconds.
             */
            long getTimeoutMillis() const;

            /**
             *
             * The timeout determines the maximum lifespan of a transaction. So if a transaction is configured with a
             * timeout of 2 minutes, then it will automatically rollback if it hasn't committed yet.
             *
             * @param timeoutInMillis  the timeout.
             * @return the updated TransactionOptions
             * @throws IllegalArgumentException if timeout smaller or equal than 0, or timeUnit is null.
             * @see #getTimeoutMillis()
             */
            TransactionOptions &setTimeout(long timeoutInMillis);

            /**
             *
             * @return the transaction durability.
             * @see #setDurability(int)
             */
            int getDurability() const;

            /**
             * Sets the transaction durability.
             *
             * The durability is the number of machines that can take over if a member fails during a transaction
             * commit or rollback. This value only has meaning when TransactionType#TWO_PHASE is selected.
             *
             * @param durability  the durability
             * @return the updated TransactionOptions.
             * @throws IllegalArgumentException if durability smaller than 0.
             */
            TransactionOptions &setDurability(int durability);

            /**
             *
             * @see DataSerializable::writeData
             */
            void writeData(serialization::ObjectDataOutput &out) const;

            /**
             *
             * @see DataSerializable::readData
             */
            void readData(serialization::ObjectDataInput &in);

        private:
            long timeoutMillis;

            int durability;

            TransactionType transactionType;

        };
    }
}

#endif //HAZELCAST_TransactionOptions

