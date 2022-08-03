#
# Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
add_executable(atomic_long atomic_long.cpp)
add_executable(fenced_lock fenced_lock.cpp)
add_executable(counting_semphore counting_semphore.cpp)
add_executable(latch latch.cpp)
add_executable(atomic_reference atomic_reference.cpp)
