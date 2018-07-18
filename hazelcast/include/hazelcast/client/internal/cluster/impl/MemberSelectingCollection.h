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
#ifndef HAZELCAST_CLIENT_INTERNAL_CLUSTER_IMPL_MEMBERSELECTINGCOLLECTION_H_
#define HAZELCAST_CLIENT_INTERNAL_CLUSTER_IMPL_MEMBERSELECTINGCOLLECTION_H_

#include <vector>

#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/util/Collection.h"
#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace cluster {
                namespace impl {
                    /**
                     * An immutable collection that applies all the {@link com.hazelcast.core.MemberSelector} instances to
                     * its internal {@link Member} collection. It reflects changes in the internal collection.
                     * It is mainly used for querying a member list.
                     * @param <M> A subclass of {@link Member} interface
                     */

                    class MemberSelectingCollection : public util::Collection<Member> {
                    public:
                        MemberSelectingCollection(const std::vector<Member> &members,
                                                  const client::cluster::memberselector::MemberSelector &selector);

                        boost::shared_ptr<util::Iterator<Member> > iterator();

                        virtual int size() const;
                    private:
                        const std::vector<Member> members;
                        const client::cluster::memberselector::MemberSelector &selector;

                        class MemberSelectingIterator : public util::Iterator<Member> {
                        public:
                            MemberSelectingIterator(const std::vector<Member> &members,
                                                    const client::cluster::memberselector::MemberSelector &selector);

                            virtual boost::shared_ptr<Member> next();

                            virtual bool hasNext();

                        private:
                            std::vector<Member>::const_iterator it;
                            const std::vector<Member>::const_iterator end;
                            const client::cluster::memberselector::MemberSelector &selector;
                            boost::shared_ptr<Member> member;
                        };
                    };
                }
            }
        }
    }
};

#endif /* HAZELCAST_CLIENT_INTERNAL_CLUSTER_IMPL_MEMBERSELECTINGCOLLECTION_H_ */
