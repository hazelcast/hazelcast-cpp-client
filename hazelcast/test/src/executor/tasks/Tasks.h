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
#pragma once

#include <stdint.h>

#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/cluster/memberselector/MemberSelectors.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace executor {
                namespace tasks {
                    enum struct task_ids {
                        APPEND_CALLABLE = 5,
                        CANCELLATION_AWARE_TASK = 6,
                        FAILING_CALLABLE = 7,
                        GET_MEMBER_UUID_TASK = 8,
                        MAP_PUTPARTITIONAWARE_CALLABLE = 9,
                        NULL_CALLABLE = 10,
                        SELECT_ALL_MEMBERS = 11,
                        SELECT_NO_MEMBERS = 12,
                        SERIALIZED_COUNTER_CALLABLE = 13,
                        TASK_WITH_UNSERIALIZABLE_RESPONSE = 14
                    };

                    struct AppendCallable {
                        std::string msg;

                        static std::string APPENDAGE;
                    };

                    struct CancellationAwareTask {
                        int64_t sleep_time;
                    };

                    struct FailingCallable {
                    };

                    struct GetMemberUuidTask {
                    };

                    template<typename T>
                    struct MapPutPartitionAwareCallable : public PartitionAware<T> {
                        const T *get_partition_key() const override {
                            return &partition_key;
                        }

                        MapPutPartitionAwareCallable(const std::string &map_name, T partition_key) :
                                map_name(map_name), partition_key(partition_key) {}

                        std::string map_name;
                        T partition_key;
                    };

                    struct NullCallable {
                    };

                    struct SelectAllMembers : public cluster::memberselector::MemberSelector {
                        bool select(const Member &member) const override;

                        void to_string(std::ostream &os) const override;
                    };

                    struct SelectNoMembers : public cluster::memberselector::MemberSelector {
                        bool select(const Member &member) const override;

                        void to_string(std::ostream &os) const override;
                    };

                    struct SerializedCounterCallable {
                        int32_t counter;
                    };

                    struct TaskWithUnserializableResponse {
                    };

                }
            }
        }

        namespace serialization {
            struct TaskSerializerFactory : public identified_data_serializer {
                static int32_t get_factory_id() {
                    return 66;
                }
            };

            template<typename T>
            struct TaskSerializerNoOpRead : public TaskSerializerFactory {
                static int32_t get_factory_id() {
                    return 66;
                }

                static T read_data(ObjectDataInput &in) {
                    return T{};
                }
            };

            template<typename T>
            struct TaskSerializerBase : public TaskSerializerNoOpRead<T> {
                static void write_data(const T &object, ObjectDataOutput &out) {}
            };

            template<>
            struct hz_serializer<test::executor::tasks::AppendCallable>
                    : public TaskSerializerNoOpRead<test::executor::tasks::AppendCallable> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::APPEND_CALLABLE);
                }

                static void write_data(const test::executor::tasks::AppendCallable &object, ObjectDataOutput &out) {
                    out.write(object.msg);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::CancellationAwareTask> : public TaskSerializerFactory  {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::CANCELLATION_AWARE_TASK);
                }

                static void write_data(const test::executor::tasks::CancellationAwareTask &object, ObjectDataOutput &out) {
                    out.write(object.sleep_time);
                }

                static test::executor::tasks::CancellationAwareTask read_data(ObjectDataInput &in) {
                    return test::executor::tasks::CancellationAwareTask{in.read<int64_t>()};
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::FailingCallable>
                    : public TaskSerializerBase<test::executor::tasks::FailingCallable> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::FAILING_CALLABLE);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::GetMemberUuidTask>
                    : public TaskSerializerBase<test::executor::tasks::GetMemberUuidTask> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::GET_MEMBER_UUID_TASK);
                }
            };

            template<typename T>
            struct hz_serializer<test::executor::tasks::MapPutPartitionAwareCallable<T>> : public TaskSerializerFactory {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::MAP_PUTPARTITIONAWARE_CALLABLE);
                }

                static void write_data(const test::executor::tasks::MapPutPartitionAwareCallable<T> &object, ObjectDataOutput &out) {
                    out.write(object.map_name);
                    out.write_object(object.partition_key);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::NullCallable>
                    : public TaskSerializerBase<test::executor::tasks::NullCallable> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::NULL_CALLABLE);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::SelectAllMembers>
                    : public TaskSerializerBase<test::executor::tasks::SelectAllMembers> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::SELECT_ALL_MEMBERS);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::SelectNoMembers>
                    : public TaskSerializerBase<test::executor::tasks::SelectNoMembers> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::SELECT_NO_MEMBERS);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::TaskWithUnserializableResponse>
                    : public TaskSerializerBase<test::executor::tasks::TaskWithUnserializableResponse> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::TASK_WITH_UNSERIALIZABLE_RESPONSE);
                }
            };

            template<>
            struct hz_serializer<test::executor::tasks::SerializedCounterCallable>
                    : public TaskSerializerNoOpRead<test::executor::tasks::SerializedCounterCallable> {
                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::executor::tasks::task_ids::SERIALIZED_COUNTER_CALLABLE);
                }

                static void write_data(const test::executor::tasks::SerializedCounterCallable &object, ObjectDataOutput &out) {
                    out.write(object.counter + 1);
                }

                static test::executor::tasks::SerializedCounterCallable read_data(ObjectDataInput &in) {
                    return test::executor::tasks::SerializedCounterCallable{in.read<int32_t>() + 1};
                }
            };
        }
    }
}

