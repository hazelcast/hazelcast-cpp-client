/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

#include "hazelcast/util/RuntimeAvailableProcessors.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace hazelcast {
    namespace util {
        util::AtomicInt RuntimeAvailableProcessors::currentAvailableProcessors(
                RuntimeAvailableProcessors::getNumberOfProcessors());

        int RuntimeAvailableProcessors::getNumberOfProcessors() {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            return sysinfo.dwNumberOfProcessors;
#else
            return sysconf(_SC_NPROCESSORS_ONLN);
#endif
        }

        int RuntimeAvailableProcessors::get() {
            return currentAvailableProcessors;
        }

        void RuntimeAvailableProcessors::override(int availableProcessors) {
            RuntimeAvailableProcessors::currentAvailableProcessors.store(availableProcessors);
        }

        void RuntimeAvailableProcessors::resetOverride() {
            currentAvailableProcessors.store(getNumberOfProcessors());
        }
    }
}

