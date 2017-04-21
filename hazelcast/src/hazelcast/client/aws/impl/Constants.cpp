/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifdef HZ_BUILD_WITH_SSL

#include "hazelcast/client/aws/impl/Constants.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                const char *Constants::DATE_FORMAT = "%Y%m%dT%H%M%SZ";
                const char *Constants::DOC_VERSION = "2014-06-15";
                const char *Constants::SIGNATURE_METHOD_V4 = "AWS4-HMAC-SHA256";
                const char *Constants::GET = "GET";
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL

