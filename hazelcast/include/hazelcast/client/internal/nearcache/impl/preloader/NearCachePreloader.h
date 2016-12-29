/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_PRELOADER_NEARCACHEPRELOADER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_PRELOADER_NEARCACHEPRELOADER_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace preloader {
                        /**
                         * Loads and stores the keys from a Near Cache into a file.
                         *
                         * @param <KS> type of the {@link NearCacheRecord} keys
                         */
                        template <typename KS>
                        class NearCachePreloader {
                            //TODO
                        public:
                            NearCachePreloader(const std::string &name,
                                               const boost::shared_ptr<config::NearCachePreloaderConfig> &config,
                                               serialization::pimpl::SerializationService &ss) {
                            }
                        };
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_PRELOADER_NEARCACHEPRELOADER_H_ */

