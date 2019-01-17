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
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#elif defined(BOOST_CLANG)
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#include <hazelcast/util/RuntimeAvailableProcessors.h>
#endif

#include "hazelcast/util/RuntimeAvailableProcessors.h"

namespace hazelcast {
    namespace util {
        util::AtomicInt RuntimeAvailableProcessors::currentAvailableProcessors(0);

        int RuntimeAvailableProcessors::getNumberOfProcessors() {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(MACOS)
            int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
            return sysconf(_SC_NPROCESSORS_ONLN);
#endif
        }

        int RuntimeAvailableProcessors::get() {
            return currentAvailableProcessors;
        }

        void RuntimeAvailableProcessors::override(int availableProcessors) {
            RuntimeAvailableProcessors::currentAvailableProcessors.set(availableProcessors);
        }
    }
}

