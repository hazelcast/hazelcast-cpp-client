/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/query/InstanceOfPredicate.h"
#include "hazelcast/client/query/impl/predicates/PredicateDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/query/ILikePredicate.h"
#include "hazelcast/client/query/TruePredicate.h"
#include "hazelcast/client/query/FalsePredicate.h"
#include "hazelcast/client/query/AndPredicate.h"
#include "hazelcast/client/query/OrPredicate.h"
#include "hazelcast/client/query/NotPredicate.h"
#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/client/query/RegexPredicate.h"
#include "hazelcast/client/query/LikePredicate.h"
#include "hazelcast/client/query/QueryConstants.h"

namespace hazelcast {
    namespace client {
        namespace query {
            InstanceOfPredicate::InstanceOfPredicate(const char *javaClassName) : className(javaClassName) {
            }

            int InstanceOfPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int InstanceOfPredicate::getClassId() const {
                return impl::predicates::INSTANCEOF_PREDICATE;
            }

            void InstanceOfPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&className);
            }

            void InstanceOfPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("InstanceOfPredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            ILikePredicate::ILikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int ILikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int ILikePredicate::getClassId() const {
                return impl::predicates::ILIKE_PREDICATE;
            }

            void ILikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void ILikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ILikePredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            int TruePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int TruePredicate::getClassId() const {
                return impl::predicates::TRUE_PREDICATE;
            }

            void TruePredicate::writeData(serialization::ObjectDataOutput &out) const {
            }

            void TruePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("TruePredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            OrPredicate::~OrPredicate() {
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    delete *it;
                }
            }

            OrPredicate &OrPredicate::add(std::unique_ptr<Predicate> &predicate) {
                return add(std::move(predicate));
            }

            OrPredicate &OrPredicate::add(std::unique_ptr<Predicate> &&predicate) {
                predicates.push_back(predicate.release());
                return *this;
            }

            int OrPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int OrPredicate::getClassId() const {
                return impl::predicates::OR_PREDICATE;
            }

            void OrPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeInt((int) predicates.size());
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    out.writeObject<serialization::IdentifiedDataSerializable>(*it);
                }
            }

            void OrPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("OrPredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            NotPredicate::NotPredicate(std::unique_ptr<Predicate> &predicate)
                    : NotPredicate::NotPredicate(std::move(predicate)) {
            }

            NotPredicate::NotPredicate(std::unique_ptr<Predicate> &&predicate)
                    : internalPredicate(std::move(predicate)) {
            }

            int NotPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int NotPredicate::getClassId() const {
                return impl::predicates::NOT_PREDICATE;
            }

            void NotPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeObject<serialization::IdentifiedDataSerializable>(internalPredicate.get());
            }

            void NotPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("NotPredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            const char *QueryConstants::getKeyAttributeName() {
                return "__key";
            }

            const char *QueryConstants::getValueAttributeName() {
                return "this";
            }

            SqlPredicate::SqlPredicate(const std::string &sqlString) : sql(sqlString) {
            }

            int SqlPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int SqlPredicate::getClassId() const {
                return impl::predicates::SQL_PREDICATE;
            }

            void SqlPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&sql);
            }

            void SqlPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("SqlPredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            int FalsePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int FalsePredicate::getClassId() const {
                return impl::predicates::FALSE_PREDICATE;
            }

            void FalsePredicate::writeData(serialization::ObjectDataOutput &out) const {
            }

            void FalsePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("FalsePredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            RegexPredicate::RegexPredicate(const char *attribute, const char *regex) : attributeName(attribute),
                                                                                       regularExpression(regex) {
            }

            int RegexPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int RegexPredicate::getClassId() const {
                return impl::predicates::REGEX_PREDICATE;
            }

            void RegexPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&regularExpression);
            }

            void RegexPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("RegexPredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            AndPredicate::~AndPredicate() {
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    delete *it;
                }
            }

            AndPredicate &AndPredicate::add(std::unique_ptr<Predicate> &predicate) {
                return add(std::move(predicate));
            }

            AndPredicate &AndPredicate::add(std::unique_ptr<Predicate> &&predicate) {
                predicates.push_back(predicate.release());
                return *this;
            }

            int AndPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int AndPredicate::getClassId() const {
                return impl::predicates::AND_PREDICATE;
            }

            void AndPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeInt((int) predicates.size());
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    out.writeObject<serialization::IdentifiedDataSerializable>(*it);
                }
            }

            void AndPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("AndPredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

            LikePredicate::LikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int LikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int LikePredicate::getClassId() const {
                return impl::predicates::LIKE_PREDICATE;
            }

            void LikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void LikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("LikePredicate::readData",
                                                                                 "Client should not need to use readData method!!!"));
            }

        }
    }
}

