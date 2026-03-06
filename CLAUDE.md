# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Hazelcast C++ client library (v5.6.0). Connects to Hazelcast distributed in-memory platform. C++11 minimum, C++14 if Boost >= 1.82.0.

## Build Commands

```bash
# Configure (from project root)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Common CMake options
cmake .. -DCMAKE_BUILD_TYPE=Debug \
  -DWITH_OPENSSL=ON \
  -DBUILD_TESTS=ON \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_SHARED_LIBS=ON

# Using the build script (run from project root)
BUILD_DIR=build BUILD_TYPE=Debug scripts/build-unix.sh -DBUILD_TESTS=ON
```

Debug builds automatically enable `-Wall -Werror -fsanitize=address`.

## Testing

Tests use Google Test and require a running Hazelcast remote controller on port 9701.

```bash
# Run all tests
cd build && ctest

# Run specific test (gtest filter)
./hazelcast/test/src/client_test --gtest_filter="MapTest.*"

# Run a single test
./hazelcast/test/src/client_test --gtest_filter="MapTest.testPutGet"
```

## Code Formatting

clang-format is enforced on PRs. Format before committing:

```bash
scripts/format-all.sh
```

Key style: 80-column limit, 4-space indent, Mozilla brace style, pointer left-aligned, no namespace indentation.

## Naming Conventions

- Classes/functions/methods: `snake_case` (e.g., `hazelcast_client`, `get_name()`)
- Member variables: trailing underscore (e.g., `alive_`, `logger_`)
- Constants: `UPPER_SNAKE_CASE`
- Namespaces: lowercase (e.g., `hazelcast::client::spi`)
- Export macros: `HAZELCAST_API` (public), `HAZELCAST_PRIVATE` (internal)

## Architecture

### Key Directories

- `hazelcast/include/hazelcast/client/` - Public API headers
- `hazelcast/src/hazelcast/client/` - Implementation (.cpp files, large monolithic files per domain)
- `hazelcast/generated-sources/` - Auto-generated protocol codecs (do not edit manually)
- `hazelcast/test/src/` - Tests (split across HazelcastTests1-8.cpp)
- `examples/` - 30+ usage examples

### Core Components

- **`hazelcast_client`** (`hazelcast_client.h`) - Main entry point, created via `new_client()` async factory
- **`client_config`** (`client_config.h`) - All client configuration
- **`ClientContext`** (`spi/ClientContext.h`) - Central context holding all internal services
- **`ClientConnectionManagerImpl`** (`network.cpp`) - Connection pooling, IO thread pool, heartbeats
- **`ClientInvocationServiceImpl`** (`spi.cpp`) - Routes requests to cluster members
- **`serialization_service`** (`serialization.cpp`) - Handles all serialization formats

### Request Flow

1. User calls API (e.g., `map.get(key)`)
2. Key serialized to `data` object
3. Partition ID computed from key
4. `ClientInvocation` created with protocol `ClientMessage`
5. Routed to connection owning the target partition
6. Response deserialized back to user type

### Threading Model

- **IO threads** - Boost.Asio io_context pool (configurable via `IO_THREAD_COUNT` property)
- **User executor** - Thread pool for callbacks/listeners
- Connections are assigned to IO threads via round-robin

### Serialization Formats

Compact serialization (with schema evolution), Portable, IdentifiedDataSerializable, custom serializers, JSON. Compact serialization implementation is in `compact.cpp` (largest source file).

## Dependencies

Managed via vcpkg. Key dependencies: Boost (asio, thread, chrono, uuid, property-tree, multiprecision), OpenSSL (optional), GTest + Thrift (tests only).
