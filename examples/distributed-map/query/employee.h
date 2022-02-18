/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <string>
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/entry_comparator.h"

namespace hazelcast {
namespace client {
namespace examples {
class employee
{
    friend serialization::hz_serializer<employee>;

public:
    employee();

    employee(std::string name, int age);

    bool operator==(const employee& employee) const;

    bool operator!=(const employee& employee) const;

    int32_t get_age() const;

    const std::string& get_name() const;

    bool operator<(const employee& rhs) const;

private:
    int32_t age_;
    std::string name_;

    // add all possible types
    byte by_;
    bool boolean_;
    char c_;
    int16_t s_;
    int32_t i_;
    int64_t l_;
    float f_;
    double d_;
    std::string str_;
    std::string utf_str_;

    std::vector<byte> byte_vec_;
    std::vector<char> cc_;
    std::vector<bool> ba_;
    std::vector<int16_t> ss_;
    std::vector<int32_t> ii_;
    std::vector<int64_t> ll_;
    std::vector<float> ff_;
    std::vector<double> dd_;
};

// Compares based on the employee age
class employee_entry_comparator
  : public query::entry_comparator<int32_t, employee>
{
public:
    int compare(
      const std::pair<const int32_t*, const employee*>* lhs,
      const std::pair<const int32_t*, const employee*>* rhs) const override;
};

// Compares based on the employee age
class employee_entry_key_comparator : public employee_entry_comparator
{
public:
    int compare(
      const std::pair<const int32_t*, const employee*>* lhs,
      const std::pair<const int32_t*, const employee*>* rhs) const override;
};

std::ostream&
operator<<(std::ostream& out, const employee& employee);

} // namespace examples
namespace serialization {
template<>
struct hz_serializer<examples::employee> : public portable_serializer
{
    static int32_t get_factory_id();

    static int32_t get_class_id();

    static void write_portable(const examples::employee& object,
                               portable_writer& writer);

    static examples::employee read_portable(portable_reader& reader);
};

template<>
struct hz_serializer<examples::employee_entry_comparator>
  : public identified_data_serializer
{
    static int32_t get_factory_id();

    static int32_t get_class_id();

    static void write_data(const examples::employee_entry_comparator& object,
                           object_data_output& writer);

    static examples::employee_entry_comparator read_data(
      object_data_input& reader);
};

template<>
struct hz_serializer<examples::employee_entry_key_comparator>
  : public identified_data_serializer
{
    static int32_t get_factory_id();

    static int32_t get_class_id();

    static void write_data(
      const examples::employee_entry_key_comparator& object,
      object_data_output& writer);

    static examples::employee_entry_key_comparator read_data(
      object_data_input& reader);
};
} // namespace serialization
} // namespace client
} // namespace hazelcast
