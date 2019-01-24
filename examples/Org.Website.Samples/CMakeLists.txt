#
# Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
add_executable(AtomicLongSample ./AtomicLongSample.cpp)
add_executable(LockSample LockSample.cpp)
add_executable(MapSample MapSample.cpp)
add_executable(MultiMapSample MultiMapSample.cpp)
add_executable(TopicSample TopicSample.cpp)
add_executable(EntryProcessorSample ./EntryProcessorSample.cpp)
add_executable(QuerySample ./QuerySample.cpp)
add_executable(QueueSample ./QueueSample.cpp)
add_executable(RingBufferReaderSample RingBufferSample.cpp)
add_executable(IdentifiedDataSerializableSample ./IdentifiedDataSerializableSample.cpp)
add_executable(PortableSerializableSample ./PortableSerializableSample.cpp)
add_executable(CustomSerializerSample ./CustomSerializerSample.cpp)
add_executable(GlobalSerializerSample ./GlobalSerializerSample.cpp)
add_executable(ListSample ./ListSample.cpp)
add_executable(SetSample ./SetSample.cpp)
add_executable(ReplicatedMapSample ./ReplicatedMapSample.cpp)
add_executable(ExecutorSample ExecutorSample.cpp)
