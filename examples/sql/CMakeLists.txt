#
# Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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
add_executable(sql_basic_query sql_basic_query.cpp)
add_executable(sql_query_with_portable sql_query_with_portable.cpp)
add_executable(sql_cancellation_example sql_cancellation_example.cpp)
add_executable(sql_json_example sql_json_example.cpp)
add_executable(sql_order_by_limit_offset sql_order_by_limit_offset.cpp)
add_executable(sql_page_iterator_sync sql_page_iterator_sync.cpp)
add_executable(sql_row_iterator_sync sql_row_iterator_sync.cpp)
add_executable(sql_advanced_query_options sql_advanced_query_options.cpp)
