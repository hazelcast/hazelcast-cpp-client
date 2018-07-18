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
#include <boost/foreach.hpp>

#include "hazelcast/client/internal/cluster/impl/MemberSelectingCollection.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace cluster {
                namespace impl {

                    MemberSelectingCollection::MemberSelectingCollection(const std::vector<Member> &members,
                                                                         const client::cluster::memberselector::MemberSelector &selector)
                            : members(members), selector(selector) {}

                    boost::shared_ptr<util::Iterator<Member> > MemberSelectingCollection::iterator() {
                        return boost::shared_ptr<util::Iterator<Member> >(new MemberSelectingIterator(members, selector));
                    }

                    int MemberSelectingCollection::size() const {
                        int size = 0;
                        BOOST_FOREACH(const Member &member , members) {
                            if (selector.select(member)) {
                                size++;
                            }
                        }

                        return size;
                    }

                    MemberSelectingCollection::MemberSelectingIterator::MemberSelectingIterator(
                            const std::vector<Member> &members,
                            const client::cluster::memberselector::MemberSelector &selector) : it(members.begin()),
                                                                                               end(members.end()),
                                                                                               selector(selector) {}

                    bool MemberSelectingCollection::MemberSelectingIterator::hasNext() {
                        while (member.get() == NULL && it != end) {
                            Member nextMember = *it;
                            if (selector.select(nextMember)) {
                                member.reset(new Member(nextMember));
                            }
                            ++it;
                        }

                        return member.get() != NULL;
                    }

                    boost::shared_ptr<Member> MemberSelectingCollection::MemberSelectingIterator::next() {
                        boost::shared_ptr<Member> nextMember;
                        if (member.get() != NULL || hasNext()) {
                            nextMember = member;
                            member.reset();
                        } else {
                            throw exception::NoSuchElementException(
                                    "MemberSelectingCollection::MemberSelectingIterator::next()");
                        }

                        return nextMember;
                    }
                }
            }
        }
    }
}

