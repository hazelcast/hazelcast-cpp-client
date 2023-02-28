#
# Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

# get git commit date and id using the git command
# if the git command fails, then date and id are set to NOT_FOUND
function(get_commit_date_and_id date id)
    execute_process(COMMAND git show -s --format=%cd OUTPUT_VARIABLE date_out RESULT_VARIABLE date_ret_val)
    execute_process(COMMAND git show -s --format=%h OUTPUT_VARIABLE id_out RESULT_VARIABLE id_ret_val)

    if (date_ret_val)
        message(WARNING "Could not execute command git show for obtaining the git commit date. Process exited with ${date_ret_val}.")
        set(date_out NOT_FOUND)
    else()
        string(STRIP ${date_out} date_out)
        if (NOT date_out)
            message(WARNING "git show command returned empty commit date.")
            set(date_out NOT_FOUND)
        endif()
    endif()

    if (id_ret_val)
        message(WARNING "Could not execute command git show for obtaining the git commit id. Process exited with ${id_ret_val}.")
        set(id_out NOT_FOUND)
    else()
        string(STRIP ${id_out} id_out)
        if (NOT id_out)
            message(WARNING "git show command returned empty commit id.")
            set(id_out NOT_FOUND)
        endif()
    endif()

    set(${date} ${date_out} PARENT_SCOPE)
    set(${id} ${id_out} PARENT_SCOPE)
endfunction()
