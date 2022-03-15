/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

/**
 * Contains the configuration for a Hazelcast transaction.
 */
class HAZELCAST_API transaction_options
{
public:
    /**
     * Transaction type.
     */
    enum struct transaction_type
    {
        TWO_PHASE = 1,
        LOCAL = 2
    };

    /**
     * Creates a new default configured TransactionsOptions.
     *
     * It will be configured with a timeout of 2 minutes, durability of 1 and a
     * TransactionType.TWO_PHASE.
     */
    transaction_options();

    /**
     *
     * @return the TransactionType.
     */
    transaction_type get_transaction_type() const;

    /**
     * Sets the TransactionType.
     *
     * A local transaction is less safe than a two phase transaction; when a
     * member fails during the commit of a local transaction, it could be that
     * some of the changes are committed, while others are not and this can
     * leave your system in an inconsistent state.
     *
     * @param transactionType the new TransactionType.
     * @return the updated TransactionOptions.
     * @see #getTransactionType()
     * @see #setDurability(int)
     */
    transaction_options& set_transaction_type(
      transaction_type transaction_type);

    /**
     *
     * @return the timeout
     */
    std::chrono::milliseconds get_timeout() const;

    /**
     *
     * The timeout determines the maximum lifespan of a transaction. So if a
     * transaction is configured with a timeout of 2 minutes, then it will
     * automatically rollback if it hasn't committed yet.
     *
     * @param timeoutInSeconds  the timeout value.
     * @return the updated TransactionOptions
     * @throws illegal_argument if timeout smaller or equal than 0, or timeUnit
     * is null.
     * @see #getTimeout()
     */
    transaction_options& set_timeout(std::chrono::milliseconds duration);

    /**
     *
     * @return the transaction durability.
     * @see #setDurability(int)
     */
    int get_durability() const;

    /**
     * Sets the transaction durability.
     *
     * The durability is the number of machines that can take over if a member
     * fails during a transaction commit or rollback. This value only has
     * meaning when TransactionType#TWO_PHASE is selected.
     *
     * @param durability  the durability
     * @return the updated TransactionOptions.
     * @throws illegal_argument if durability smaller than 0.
     */
    transaction_options& set_durability(int num_machines);

private:
    std::chrono::milliseconds timeout_;
    int durability_;
    transaction_type transaction_type_;
};
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
