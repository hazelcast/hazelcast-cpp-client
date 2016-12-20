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
// Created by ihsan demir on 22 Nov 2016.

#include "hazelcast/client/config/EvictionConfig.h"
#include "hazelcast/util/Preconditions.h"

namespace hazelcast {
    namespace client {
        namespace config {
            EvictionConfig::EvictionConfig() : size(DEFAULT_MAX_ENTRY_COUNT), maxSizePolicy(DEFAULT_MAX_SIZE_POLICY),
                               evictionPolicy(DEFAULT_EVICTION_POLICY), sizeConfigured(false) {
            }

            /**
             * Default maximum entry count.
             */
            const int EvictionConfig::DEFAULT_MAX_ENTRY_COUNT = 10000;

            /**
             * Default maximum entry count for Map on-heap Near Caches.
             */
            const int32_t EvictionConfig::DEFAULT_MAX_ENTRY_COUNT_FOR_ON_HEAP_MAP = INT32_MAX;

            /**
             * Default Max-Size Policy.
             */
            const EvictionConfig::MaxSizePolicy EvictionConfig::DEFAULT_MAX_SIZE_POLICY = ENTRY_COUNT;

            /**
             * Default Eviction Policy.
             */
            const EvictionPolicy EvictionConfig::DEFAULT_EVICTION_POLICY = LRU;

            int EvictionConfig::getSize() const {
                return size;
            }

            EvictionConfig &EvictionConfig::setSize(int size) {
                this->sizeConfigured = true;
                this->size = util::Preconditions::checkPositive(size, "Size must be positive number!");
                return *this;
            }

            EvictionPolicy EvictionConfig::getEvictionPolicy() const {
                return evictionPolicy;
            }

            EvictionConfig &EvictionConfig::setMaximumSizePolicy(const EvictionConfig::MaxSizePolicy &maxSizePolicy) {
                this->maxSizePolicy = maxSizePolicy;
                return *this;
            }

            std::ostream &operator<<(std::ostream &out, const EvictionConfig &config) {
                //TODO
                return out;
            }
        }
    }
}

