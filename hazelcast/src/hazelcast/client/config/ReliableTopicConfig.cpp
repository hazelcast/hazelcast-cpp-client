/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 9 June 2016.

#include <hazelcast/client/exception/IllegalArgumentException.h>
#include <hazelcast/client/protocol/ClientProtocolErrorCodes.h>
#include "hazelcast/client/config/ReliableTopicConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {
            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() {

            }

            ReliableTopicConfig::ReliableTopicConfig(const char *topicName) : readBatchSize(DEFAULT_READ_BATCH_SIZE),
                                                                              name(topicName) {
            }

            const std::string &ReliableTopicConfig::getName() const {
                return name;
            }

            int ReliableTopicConfig::getReadBatchSize() const {
                return readBatchSize;
            }

            ReliableTopicConfig &ReliableTopicConfig::setReadBatchSize(int batchSize) {
                if (batchSize <= 0) {
                    throw exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                              "readBatchSize should be positive",
                                                              protocol::ILLEGAL_ARGUMENT, -1);
                }

                this->readBatchSize = batchSize;

                return *this;
            }
        }
    }
}
