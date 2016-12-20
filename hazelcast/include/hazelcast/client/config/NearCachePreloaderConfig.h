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
#ifndef HAZELCAST_CLIENT_CONFIG_NEARCACHEPRELOADERCONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_NEARCACHEPRELOADERCONFIG_H_

#include "hazelcast/util/HazelcastDll.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * @Beta
             * @BinaryInterface
             * Configuration for eviction.
             * You can set a limit for number of entries or total memory cost of entries.
             */
            class HAZELCAST_API NearCachePreloaderConfig {
            public:
/*
                */
/**
                 * Default initial delay for the Near Cache key storage.
                 *//*

            public static final int DEFAULT_STORE_INITIAL_DELAY_SECONDS = 600;

                */
/**
                 * Default interval for the Near Cache key storage (in seconds).
                 *//*

            public static final int DEFAULT_STORE_INTERVAL_SECONDS = 600;

            private boolean enabled;
            private String fileName = "";
            private int storeInitialDelaySeconds = DEFAULT_STORE_INITIAL_DELAY_SECONDS;
            private int storeIntervalSeconds = DEFAULT_STORE_INTERVAL_SECONDS;

            private NearCachePreloaderConfig readOnly;

            public NearCachePreloaderConfig() {
                }

            public NearCachePreloaderConfig(NearCachePreloaderConfig nearCachePreloaderConfig) {
                    */
/**
                     * ===== NOTE =====
                     *
                     * Do not use setters, because they are overridden in the readonly version of this config and
                     * they cause an "UnsupportedOperationException". Just set directly if the value is valid.
                     *//*


                    this(nearCachePreloaderConfig.enabled, nearCachePreloaderConfig.fileName);
                }

            public NearCachePreloaderConfig(String fileName) {
                    */
/**
                     * ===== NOTE =====
                     *
                     * Do not use setters, because they are overridden in the readonly version of this config and
                     * they cause an "UnsupportedOperationException". Just set directly if the value is valid.
                     *//*


                    this(true, fileName);
                }

            public NearCachePreloaderConfig(boolean enabled, String fileName) {
                    */
/**
                     * ===== NOTE =====
                     *
                     * Do not use setters, because they are overridden in the readonly version of this config and
                     * they cause an "UnsupportedOperationException". Just set directly if the value is valid.
                     *//*


                    this.enabled = enabled;
                    this.fileName = checkNotNull(fileName, "fileName cannot be null!");
                }

                NearCachePreloaderConfig getAsReadOnly() {
                    if (readOnly == null) {
                        readOnly = new NearCachePreloaderConfigReadOnly(this);
                    }
                    return readOnly;
                }

            public bool isEnabled() const {
                    return enabled;
                }

            public NearCachePreloaderConfig setEnabled(boolean isEnabled) {
                    this.enabled = isEnabled;
                    return this;
                }

            public NearCachePreloaderConfig setFileName(String fileName) {
                    this.fileName = checkNotNull(fileName, "fileName cannot be null!");
                    return this;
                }

            public String getFileName() {
                    return fileName;
                }

            public int getStoreInitialDelaySeconds() {
                    return storeInitialDelaySeconds;
                }

            public NearCachePreloaderConfig setStoreInitialDelaySeconds(int storeInitialDelaySeconds) {
                    this.storeInitialDelaySeconds = checkPositive(storeInitialDelaySeconds,
                                                                  "storeInitialDelaySeconds must be a positive number!");
                    return this;
                }

            public int getStoreIntervalSeconds() {
                    return storeIntervalSeconds;
                }

            public NearCachePreloaderConfig setStoreIntervalSeconds(int storeIntervalSeconds) {
                    this.storeIntervalSeconds = checkPositive(storeIntervalSeconds,
                                                              "storeIntervalSeconds must be a positive number!");
                    return this;
                }

                @Override
            public String toString() {
                    return "NearCachePreloaderConfig{"
                           + "enabled=" + enabled
                           + ", fileName=" + fileName
                           + ", storeInitialDelaySeconds=" + storeInitialDelaySeconds
                           + ", storeIntervalSeconds=" + storeIntervalSeconds
                           + '}';
                }

                */
/**
                 * A readonly version of the {@link NearCachePreloaderConfig}.
                 *//*

                @Beta
                @PrivateApi
                @BinaryInterface
            private static class NearCachePreloaderConfigReadOnly extends NearCachePreloaderConfig {

                    NearCachePreloaderConfigReadOnly(NearCachePreloaderConfig nearCachePreloaderConfig) {
                        super(nearCachePreloaderConfig);
                    }

                    @Override
                    public NearCachePreloaderConfig setEnabled(boolean isEnabled) {
                        throw new UnsupportedOperationException();
                    }

                    @Override
                    public NearCachePreloaderConfig setFileName(String fileName) {
                        throw new UnsupportedOperationException();
                    }

                    @Override
                    public NearCachePreloaderConfig setStoreInitialDelaySeconds(int storeInitialDelaySeconds) {
                        throw new UnsupportedOperationException();
                    }

                    @Override
                    public NearCachePreloaderConfig setStoreIntervalSeconds(int storeIntervalSeconds) {
                        throw new UnsupportedOperationException();
                    }
                }
*/
            };

            std::ostream &operator<<(std::ostream &out, const NearCachePreloaderConfig &config);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_CONFIG_NEARCACHEPRELOADERCONFIG_H_ */
