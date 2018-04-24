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

#include <vector>

#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            int BuildInfo::calculateVersion(const std::string &version) {
                std::vector<std::string> versionTokens = util::StringUtil::tokenizeVersionString(version);
                if (!versionTokens.empty()) {
                    try {
                        int calculatedVersion = MAJOR_VERSION_MULTIPLIER * util::IOUtil::to_value<int>(versionTokens[0])
                                                + MINOR_VERSION_MULTIPLIER *
                                                  util::IOUtil::to_value<int>(versionTokens[1]);
                        if (versionTokens.size() > PATCH_TOKEN_INDEX) {
                            size_t snapshotStartPosition = versionTokens[PATCH_TOKEN_INDEX].find("-");
                            if (snapshotStartPosition == std::string::npos) {
                                calculatedVersion += util::IOUtil::to_value<int>(versionTokens[PATCH_TOKEN_INDEX]);
                            }

                            if (snapshotStartPosition > 0) {
                                calculatedVersion += util::IOUtil::to_value<int>(
                                        versionTokens[PATCH_TOKEN_INDEX].substr(0, snapshotStartPosition));
                            }
                        }
                        return calculatedVersion;
                    } catch (exception::IException &e) {
                        util::ILogger::getLogger().warning() << "Failed to calculate version using version string "
                                                             << version << e;
                    }
                }

                return UNKNOWN_HAZELCAST_VERSION;
            }
        }
    }
}
