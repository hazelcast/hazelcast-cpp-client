# Configurable IO Thread Count for Hazelcast C++ Client SDK

**Date**: 2026-03-05
**Status**: Approved
**Repo**: hazelcast-cpp-client
**PR**: #1410

## Problem

The Hazelcast C++ client SDK uses a single `io_thread_` running one `boost::asio::io_context` for ALL socket I/O across
all member connections. This creates a throughput ceiling:

- **C++ benchmark**: ~40k ops/sec, optimal at 80-160 threads (saturates immediately)
- **Java benchmark**: ~450k ops/sec, scales to 480+ threads
- **Gap**: 10-13x lower throughput

The single IO thread serializes all network reads/writes for all connections (typically 3 in a 3-member cluster). Adding
more benchmark threads beyond ~80 provides zero throughput improvement because they all funnel through one event loop.

### Lab Evidence

| Client | PUT 4B ops/sec | GET 4B ops/sec | Optimal Threads |
|--------|----------------|----------------|-----------------|
| C++    | 40,489         | 46,963         | 130-160         |
| Java   | 446,399        | 604,354        | 480-485         |

Results from: `lab-results/run-20260227-141935-hz5.6.0_cpp_15min` and
`lab-results/run-20260227-203659-hz5.6.0_java_15min` (40-core lab machine, 3-member cluster). The data files are
available at [benchmark results repo](https://github.com/hazelcast/hazelcast-client-benchmark-results/tree/main/cpp).
The report can be found
at [here](https://hazelcast.github.io/hazelcast-client-benchmark-results/cpp/run-20260228-040244-hz5.6.0_cpp_30min/benchmark-report.html).

## Java Client IO Threading Model (Reference)

The Java client uses three separate thread categories for network I/O:

1. **IO Input Threads** (`hazelcast.client.io.input.thread.count`, default 3 for smart clients with >8 cores): NIO
   selector threads handling `OP_READ` — read bytes, decode frames
2. **IO Output Threads** (`hazelcast.client.io.output.thread.count`, default 3): NIO selector threads handling
   `OP_WRITE` — serialize and send frames
3. **Response Threads** (`hazelcast.client.response.thread.count`, default 2): Process decoded responses — look up
   invocations by correlation ID, complete futures

The input/output separation exists because Java NIO uses a **reactor pattern** where `selector.select()` blocks. A
thread blocked waiting for write readiness can't process reads, and vice versa.

### Why C++ Uses a Unified IO Thread Model

The C++ client uses Boost.Asio, which implements the **proactor pattern**:

- `async_write()` and `async_read_some()` both return immediately (non-blocking)
- Completions of both are delivered to the same `io_context` event loop
- Neither operation blocks the other — there is no mutual exclusion between read and write processing
- A socket is permanently bound to one `io_context` at construction; you cannot route reads to one executor and writes
  to another

Separating input/output threads in Asio would require either: (a) two io_contexts per socket via `dup()` file descriptor
duplication (platform-specific, fragile), or (b) separate read/write strands on a multi-threaded io_context (breaks the
unsynchronized `Connection::invocations` map). Neither is idiomatic or beneficial.

**3 unified Asio IO threads provide equivalent parallelism to Java's 3 input + 3 output threads** — each Asio thread
handles both directions simultaneously without blocking, while each Java thread handles only one direction.

## Solution

Two new client properties:

### 1. IO Thread Count

Replace the single `io_context_`/`io_thread_` with N io_contexts, each with its own thread. Connections are assigned to
io_contexts via round-robin.

- **Name**: `hazelcast.client.io.thread.count`
- **Default**: `"3"` (equivalent to Java's 3 input + 3 output threads)
- **Type**: Integer (>= 1)
- **Resolution priority**: `client_config::set_property()` > environment variable > default

This is the C++ equivalent of Java's `IO_INPUT_THREAD_COUNT` + `IO_OUTPUT_THREAD_COUNT`, unified because Asio's proactor
handles both directions per thread.

### 2. Response Thread Count

Configure the `user_executor_` pool size for response processing (future `.then()` continuations).

- **Name**: `hazelcast.client.response.thread.count`
- **Default**: `"2"` (matches Java's `ClientProperty.RESPONSE_THREAD_COUNT`)
- **Type**: Integer (>= 1)
- **Resolution priority**: `client_config::set_property()` > environment variable > default
- **Replaces**: The existing `client_config::set_executor_pool_size()` path for `user_executor_` sizing. If
  `RESPONSE_THREAD_COUNT` is set, it takes priority. If not set and `executor_pool_size` is set via `client_config`,
  that value is used. Otherwise default 2.

### Usage

```cpp
// Via API:
config.set_property("hazelcast.client.io.thread.count", "4");
config.set_property("hazelcast.client.response.thread.count", "3");

// Via environment variables:
// export hazelcast.client.io.thread.count=4
// export hazelcast.client.response.thread.count=3
```

## Architecture

### Before (single IO thread)

```
N benchmark threads
  -> map->put(key, val).get()  (blocking)
  -> 3 TCP connections (1 per member, smart routing)
  -> 1 io_context_ / 1 io_thread_   <-- BOTTLENECK
```

### After (configurable IO threads)

```
N benchmark threads
  -> map->put(key, val).get()  (blocking)
  -> 3 TCP connections, round-robin assigned:
     -> io_contexts_[0] / io_threads_[0]  (connection to member A)
     -> io_contexts_[1] / io_threads_[1]  (connection to member B)
     -> io_contexts_[2] / io_threads_[2]  (connection to member C)
```

Each connection's `socket_strand_` serializes operations within that connection, running on its assigned `io_context`'s
thread. No new synchronization is needed because:

- `Connection::invocations` (no mutex) is protected by the strand — still correct with multiple io_contexts
- `active_connections_` is a `SynchronizedMap` — already thread-safe
- `HeartbeatManager` uses `internal_executor_` (separate pool) — unaffected
- Event/listener dispatch uses `event_executor_` (separate pool) — unaffected

### Why Default 3 (Not Hardware-Dependent)

- IO threads are **IO-bound, not CPU-bound** — one thread can saturate a 10Gbps NIC
- The bottleneck is serialization across connections, not CPU on the IO thread
- With 3 members (typical), 3 IO threads = zero cross-connection serialization
- More IO threads than connections = idle threads with no benefit
- Java uses the same default of 3 for the same reasons
- Users with larger clusters can tune upward via the property

## Files to Modify

All paths relative to the hazelcast-cpp-client repo root.

### 1. `hazelcast/include/hazelcast/client/client_properties.h`

Add following the existing pattern (e.g., `INTERNAL_EXECUTOR_POOL_SIZE`):

```cpp
    /**
     * Number of IO threads for the networking layer.
     * Each IO thread runs its own Boost.Asio io_context event loop.
     * Connections are distributed across IO threads in round-robin fashion.
     * This is the C++ equivalent of Java's IO_INPUT_THREAD_COUNT +
     * IO_OUTPUT_THREAD_COUNT (unified because Asio's proactor handles both
     * read and write directions per thread).
     */
    static const std::string IO_THREAD_COUNT;
    static const std::string IO_THREAD_COUNT_DEFAULT;

    /**
     * Number of threads for processing responses (completing futures).
     * Matches Java's ClientProperty.RESPONSE_THREAD_COUNT.
     * Controls the user_executor_ pool that runs future .then() continuations.
     */
    static const std::string RESPONSE_THREAD_COUNT;
    static const std::string RESPONSE_THREAD_COUNT_DEFAULT;

    // Public accessors:
    const client_property& get_io_thread_count() const;
    const client_property& get_response_thread_count() const;

    // Private fields:
    client_property io_thread_count_;
    client_property response_thread_count_;
```

### 2. `hazelcast/src/hazelcast/client/client_impl.cpp`

Add string definitions:

```cpp
const std::string client_properties::IO_THREAD_COUNT =
  "hazelcast.client.io.thread.count";
const std::string client_properties::IO_THREAD_COUNT_DEFAULT = "3";

const std::string client_properties::RESPONSE_THREAD_COUNT =
  "hazelcast.client.response.thread.count";
const std::string client_properties::RESPONSE_THREAD_COUNT_DEFAULT = "2";
```

Add to `client_properties` constructor initializer list:

```cpp
  , io_thread_count_(IO_THREAD_COUNT, IO_THREAD_COUNT_DEFAULT)
  , response_thread_count_(RESPONSE_THREAD_COUNT, RESPONSE_THREAD_COUNT_DEFAULT)
```

Add accessor implementations:

```cpp
const client_property&
client_properties::get_io_thread_count() const
{
    return io_thread_count_;
}

const client_property&
client_properties::get_response_thread_count() const
{
    return response_thread_count_;
}
```

### 2b. `hazelcast/src/hazelcast/client/spi.cpp` (ClientExecutionServiceImpl)

Update `start()` to use `RESPONSE_THREAD_COUNT` for `user_executor_` sizing:

```cpp
void ClientExecutionServiceImpl::start()
{
    // ... internal_executor_ unchanged ...

    // user_executor_: use RESPONSE_THREAD_COUNT property, fall back to
    // executor_pool_size from client_config, then default 2
    int responseThreadCount = client_properties_.get_integer(
        client_properties_.get_response_thread_count());
    if (responseThreadCount <= 0) {
        responseThreadCount = util::IOUtil::to_value<int>(
            client_properties::RESPONSE_THREAD_COUNT_DEFAULT);
    }
    if (user_pool_size_ > 0) {
        // Explicit executor_pool_size from client_config takes precedence
        user_executor_.reset(
            new hazelcast::util::hz_thread_pool(user_pool_size_));
    } else {
        user_executor_.reset(
            new hazelcast::util::hz_thread_pool(responseThreadCount));
    }

    // ... schema_replication_executor_ unchanged ...
}
```

### 3. `hazelcast/include/hazelcast/client/connection/ClientConnectionManagerImpl.h`

Replace single io_context/io_thread with vectors:

```cpp
// Before:
std::unique_ptr<boost::asio::io_context> io_context_;
std::thread io_thread_;
std::unique_ptr<boost::asio::executor_work_guard<
    boost::asio::io_context::executor_type>> io_guard_;
std::unique_ptr<boost::asio::ip::tcp::resolver> io_resolver_;

// After:
std::vector<std::unique_ptr<boost::asio::io_context>> io_contexts_;
std::vector<std::thread> io_threads_;
std::vector<std::unique_ptr<boost::asio::executor_work_guard<
    boost::asio::io_context::executor_type>>> io_guards_;
std::vector<std::unique_ptr<boost::asio::ip::tcp::resolver>> io_resolvers_;
std::atomic<size_t> next_io_index_{0};
```

Add helper method:

```cpp
// Returns the next io_context + resolver pair via round-robin
std::pair<boost::asio::io_context&, boost::asio::ip::tcp::resolver&>
    next_io_context();
```

### 4. `hazelcast/include/hazelcast/client/internal/socket/SocketFactory.h`

Change `create()` to accept io_context and resolver per call instead of storing a single reference:

```cpp
// Before:
SocketFactory(spi::ClientContext& client_context,
              boost::asio::io_context& io,
              boost::asio::ip::tcp::resolver& resolver);

std::unique_ptr<hazelcast::client::socket> create(
    const address& address,
    std::chrono::milliseconds& connect_timeout_in_millis);

// After:
SocketFactory(spi::ClientContext& client_context);

std::unique_ptr<hazelcast::client::socket> create(
    const address& address,
    std::chrono::milliseconds& connect_timeout_in_millis,
    boost::asio::io_context& io,
    boost::asio::ip::tcp::resolver& resolver);
```

### 5. `hazelcast/src/hazelcast/client/network.cpp`

**`ClientConnectionManagerImpl::start()`** — create N io_contexts:

```cpp
auto& props = client_.get_client_properties();
int io_thread_count = props.get_integer(props.get_io_thread_count());
if (io_thread_count <= 0) {
    io_thread_count = util::IOUtil::to_value<int>(
        client_properties::IO_THREAD_COUNT_DEFAULT);
}

socket_factory_.reset(new internal::socket::SocketFactory(client_));

for (int i = 0; i < io_thread_count; ++i) {
    auto ctx = std::make_unique<boost::asio::io_context>();
    auto guard = boost::asio::make_work_guard(*ctx);
    io_resolvers_.push_back(
        std::make_unique<boost::asio::ip::tcp::resolver>(ctx->get_executor()));
    io_guards_.push_back(std::make_unique<...>(std::move(guard)));
    io_threads_.emplace_back([raw = ctx.get()]() { raw->run(); });
    io_contexts_.push_back(std::move(ctx));
}
```

**`ClientConnectionManagerImpl::shutdown()`** — join all:

```cpp
for (auto& guard : io_guards_) guard.reset();
for (auto& thread : io_threads_) thread.join();
io_contexts_.clear();
io_guards_.clear();
io_resolvers_.clear();
io_threads_.clear();  // already joined
```

**Connection creation** — use round-robin io_context:

```cpp
auto [io_ctx, resolver] = next_io_context();
// pass io_ctx and resolver to socket_factory_->create(...)
```

**`SocketFactory::create()`** — use the provided io_context:

```cpp
std::unique_ptr<hazelcast::client::socket>
SocketFactory::create(const address& address,
                      std::chrono::milliseconds& connect_timeout_in_millis,
                      boost::asio::io_context& io,
                      boost::asio::ip::tcp::resolver& resolver)
{
#ifdef HZ_BUILD_WITH_SSL
    if (ssl_context_.get()) {
        return std::make_unique<internal::socket::SSLSocket>(
            io, *ssl_context_, address, socket_options_,
            connect_timeout_in_millis, resolver);
    }
#endif
    return std::make_unique<internal::socket::TcpSocket>(
        io, address, socket_options_,
        connect_timeout_in_millis, resolver);
}
```

## Unit Tests

Add to `hazelcast/test/src/` (Google Test):

### IO Thread Count Tests

| Test                                      | What It Verifies                                                                                           |
|-------------------------------------------|------------------------------------------------------------------------------------------------------------|
| `IoThreadCountDefaultTest`                | Default property value is `"3"`                                                                            |
| `IoThreadCountCustomPropertyTest`         | `set_property("hazelcast.client.io.thread.count", "5")` is respected                                       |
| `IoThreadCountSingleTest`                 | `set_property(..., "1")` works (backward compat with single IO thread)                                     |
| `IoThreadCountInvalidTest`                | Value `"0"` or `"-1"` falls back to default 3                                                              |
| `IoThreadCountConnectionDistributionTest` | Connections to 3 members are distributed across io_contexts (integration test requiring a running cluster) |

### Response Thread Count Tests

| Test                                    | What It Verifies                                                           |
|-----------------------------------------|----------------------------------------------------------------------------|
| `ResponseThreadCountDefaultTest`        | Default property value is `"2"`                                            |
| `ResponseThreadCountCustomTest`         | `set_property("hazelcast.client.response.thread.count", "4")` is respected |
| `ResponseThreadCountSetViaClientConfig` | `set_property()` stores correctly in config                                |

## Verification Plan

1. Build the modified C++ client SDK locally
2. Point the benchmark project's vcpkg to the local build (overlay port or direct cmake reference)
3. Run `hzbench run max-throughput PUT 4 5 --client-type cpp` locally
4. Compare throughput with the baseline (~40k ops/sec with 1 IO thread)
5. If local results improve, run full lab suite for definitive comparison

## Risks and Mitigations

| Risk                                                        | Mitigation                                                                                                       |
|-------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| SSL context sharing across io_contexts                      | SSL context is thread-safe in OpenSSL; each socket gets its own SSL stream. Verify with `WITH_OPENSSL=ON` build. |
| Existing tests break                                        | Default changes from 1->3 IO threads but the API is unchanged. Run full test suite.                              |
| Connection to member happens before round-robin index wraps | Round-robin via atomic increment + modulo is safe for any number of connections vs io_contexts.                  |
