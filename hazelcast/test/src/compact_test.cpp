/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "compact/compact_serialization_test.h"
#include "compact/compact_rabin_fingerprint_test.h"
#include "compact/compact_nullable_primitive_interoperability_test.h"
#include "compact/compact_schema_replication_on_write_test.h"
#include "compact/compact_schema_replication_on_cluster_restart_test.h"
#include "compact/compact_schema_validation_test.h"
#include "compact/compact_schema_replication_stress_test.h"
#include "compact/compact_schema_fetch_on_read.h"
#include "compact/compact_read_write_integration_test.h"
#include "compact/compact_field_kind_test.h"
#include "compact/compact_generic_record_builder_test.h"
#include "compact/compact_generic_record_test.h"
#include "compact/compact_generic_record_integration_test.h"