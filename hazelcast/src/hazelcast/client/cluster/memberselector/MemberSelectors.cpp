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
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace memberselector {
                bool MemberSelectors::DataMemberSelector::select(const Member &member) const {
                    return !member.isLiteMember();
                }

                void MemberSelectors::DataMemberSelector::toString(std::ostream &os) const {
                    os << "Default DataMemberSelector";
                }

                const std::unique_ptr<MemberSelector> MemberSelectors::DATA_MEMBER_SELECTOR(
                        new MemberSelectors::DataMemberSelector());
            }
        }
    }
}
