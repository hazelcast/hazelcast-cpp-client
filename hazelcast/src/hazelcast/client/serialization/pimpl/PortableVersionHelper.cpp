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
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"

#include "hazelcast/client/serialization/VersionedPortable.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                int PortableVersionHelper::getVersion(const Portable *portable, int defaultVersion) {
                    int version = defaultVersion;
                    if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                        version = versionedPortable->getClassVersion();
                        if (version < 0) {
                            throw exception::IllegalArgumentException("PortableVersionHelper:getVersion", "Version cannot be negative!");
                        }
                    }
                    return version;
                }
            }
        }
    }
}
