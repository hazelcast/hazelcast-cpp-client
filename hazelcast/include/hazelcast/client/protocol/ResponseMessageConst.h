/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

namespace hazelcast {
    namespace client {
        namespace protocol {
            HAZELCAST_API enum ResponseMessageConst {
                //VOID = 100,
                BOOLEAN = 101,
                INTEGER = 102,
                LONG = 103,
                STRING = 104,
                DATA = 105,
                LIST_DATA = 106,
                AUTHENTICATION = 107,
                PARTITIONS = 108,
                EXCEPTION = 109,
                SET_DISTRIBUTED_OBJECT = 110,
                ENTRY_VIEW = 111,
                JOB_PROCESS_INFO = 112,
                SET_DATA = 113,
                SET_ENTRY = 114,
                READ_RESULT_SET = 115,
                CACHE_KEY_ITERATOR_RESULT = 116,
                LIST_ENTRY = 117
            };
        }
    }
}



