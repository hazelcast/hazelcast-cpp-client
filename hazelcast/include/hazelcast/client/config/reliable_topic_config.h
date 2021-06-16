/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace config {
class HAZELCAST_API reliable_topic_config
{
public:
    static constexpr int DEFAULT_READ_BATCH_SIZE = 10;

    reliable_topic_config();

    explicit reliable_topic_config(std::string topic_name);

    /**
     * Gets the name of the reliable topic.
     *
     * @return the name of the reliable topic.
     */
    const std::string& get_name() const;

    /**
     * Gets the maximum number of items to read in a batch. Returned value will always be equal or
     * larger than 1.
     *
     * @return the read batch size.
     */
    int get_read_batch_size() const;

    /**
     * Sets the read batch size.
     *
     * The ReliableTopic tries to read a batch of messages from the ringbuffer. It will get at least
     * one, but if there are more available, then it will try to get more to increase throughput.
     * The minimal read batch size can be influenced using the read batch size.
     *
     *
     * If the readBatchSize is 10 and there are 50 messages available, 10 items are retrieved and
     * processed consecutively.
     *
     * If the readBatchSize is 10 and there are 2 items available, 2 items are retrieved and
     * processed consecutively.
     *
     *
     * @param readBatchSize the maximum number of items to read in a batch.
     * @return the updated reliable topic config.
     * @throws illegal_argument if readBatchSize is smaller than 1.
     */
    reliable_topic_config& set_read_batch_size(int batch_size);

private:
    int read_batch_size_;
    std::string name_;
};
} // namespace config
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
