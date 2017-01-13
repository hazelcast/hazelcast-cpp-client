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
//
// Created by ihsan demir on 14 Dec 2016.
//

#include "hazelcast/client/map/impl/nearcache/KeyStateMarker.h"

namespace hazelcast {
    namespace client {
        namespace map{
            namespace impl {
                namespace nearcache {
                    bool TrueMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    void TrueMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                    }

                    void TrueMarkerImpl::init() {
                    }

                    const std::auto_ptr<KeyStateMarker> KeyStateMarker::TRUE_MARKER =
                            std::auto_ptr<KeyStateMarker>(new TrueMarkerImpl());
                }
            }
        }
    }
}

