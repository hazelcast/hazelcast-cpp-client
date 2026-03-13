# C++ Client Pipeline Rearchitecture Design

**Date:** 2026-03-05 \
**Status:** Approved \
**Base branch:** `fixIoThreadCount` ([PR](https://github.com/hazelcast/hazelcast-cpp-client/pull/144)) \
**Goal:** Close the 4.3x throughput gap (103K vs 446K ops/sec PUT 4B) by aligning C++ client networking architecture
with Java client.

## Problem

After the IO thread count fix ([PR](https://github.com/hazelcast/hazelcast-cpp-client/pull/144)), C++ client throughput plateaus at ~103K ops/sec for PUT 4B values with 240
threads. Java client achieves ~446K ops/sec with 485 threads on the same hardware. The gap is caused by architectural
differences in the networking pipeline.

### Root Cause Analysis

| Bottleneck                                  | C++ Current                                           | Java                                                                        | Impact                                         |
|---------------------------------------------|-------------------------------------------------------|-----------------------------------------------------------------------------|------------------------------------------------|
| Response processing on IO thread            | `set_value()` blocks IO strand                        | ResponseThread pool offloads from IO                                        | IO thread can't read while completing promises |
| Per-connection strand serializes everything | Reads, writes, map ops all on 1 strand per connection | Separate input/output selectors + response threads                          | 240 caller threads contend on 3 strands        |
| Per-connection invocations map              | `std::unordered_map` accessed only from strand        | Global `ConcurrentHashMap` accessed lock-free                               | All map ops serialized through strand          |
| No write batching                           | Each put = separate `async_write` syscall             | `ConcurrentLinkedQueue` drained into buffer, single `socketChannel.write()` | N syscalls vs 1                                |
| Read buffer undersized                      | 16 KB hardcoded                                       | 128 KB from socket options                                                  | More read syscalls needed                      |

## Design

### 1. Global Invocation Registry

Replace per-connection `std::unordered_map<int64_t, shared_ptr<ClientInvocation>>` on `Connection` with a single global
`boost::concurrent_flat_map<int64_t, shared_ptr<ClientInvocation>>` on `ClientInvocationServiceImpl`.

- `boost::concurrent_flat_map` is available in Boost 1.83+ (project uses 1.89)
- Uses open-addressing with SIMD probing, visitor-based API
- Zero new dependencies — just add `boost-unordered` to vcpkg.json
- Existing `CallIdSequence` implementations and correlation ID generation (connection_id in upper 32 bits) are preserved
  unchanged
- Correlation IDs are globally unique so the global map lookup works as-is

**Registration flow (mirrors Java `ClientInvocationServiceImpl.registerInvocation()`):**

1. Caller thread: `call_id_sequence_->next()` (back pressure, existing)
2. Caller thread: `generate_new_call_id(connection)` (existing scheme)
3. Caller thread: `message->set_correlation_id(id)`
4. Caller thread: `invocations_.insert_or_assign(id, invocation)` on global concurrent map
5. Caller thread: enqueue to connection's write queue
6. If write fails: `invocations_.erase(id)`, notify exception

### 2. Lock-Free Write Queue with Batch Flushing

Replace strand-posted writes with a lock-free MPSC queue per connection, drained by the IO thread into a send buffer and
flushed as a single syscall.

**Write queue entry:**

```cpp
struct OutboundEntry {
    int64_t correlation_id;          // for error path lookup in global map
    protocol::ClientMessage message; // serialized message to write
};
```

**Queue:** `boost::lockfree::queue<OutboundEntry*>` per connection. Caller `new`s the entry, IO thread (single consumer)
takes ownership and `delete`s after processing. Raw pointers are trivially copyable as required by `boost::lockfree`.

**Send buffer:** 128 KB application-level buffer per connection, size from
`socket_options::get_buffer_size_in_bytes()` (matches Java's default). IO thread fills buffer from drained queue
entries, then does one `async_write`.

**Enqueue (caller thread, lock-free):**

```
connection->enqueue_write(new OutboundEntry{id, message})
    write_queue_.push(entry)
    post flush signal to io_context
```

**Flush (IO thread, on strand):**

```
flush_write_queue():
    drain write_queue_ into local batch (pop all)
    for each entry in local batch:
        copy frame bytes into send_buffer_
        if send_buffer_ full: break (come back for remainder)
        delete entry
    async_write(socket_, buffer(send_buffer_, filled_size),
        strand_.wrap(on_complete))

on_complete:
    if has_remainder or write_queue_ not empty:
        flush_write_queue()
```

**IO thread wake-up:** After enqueuing, caller posts a lightweight flush notification to the connection's `io_context`
via `boost::asio::post()`. If a flush/write is already in progress, the completion handler checks for more queued
messages.

### 3. Response Thread Pool

Add a response thread pool that handles invocation completion off the IO thread.

**Thread pool:** N response threads (default 2, configurable via existing `hazelcast.client.response.thread.count`
property). Each thread runs a loop pulling from its own queue using `std::mutex` + `std::condition_variable` for
efficient blocking when empty.

**IO thread handoff (mirrors Java `ClientResponseHandlerSupplier`):**

- IO thread reads and decodes message (must stay on IO thread — owns socket)
- Extracts correlation ID, classifies message type:
    - **Response:** enqueue `{correlationId, message}` to `response_queues_[correlationId % N]`, return immediately
    - **Backup event:** handle inline (cheap — just decrement counter, same as Java)
    - **Listener event:** route to listener service (existing behavior)

**Response thread work:**

```
loop:
    entry = queue.pop()              // blocks if empty
    invocations_.visit(entry.id, [&](auto& pair) {
        invocation = pair.second
    })
    if not found: discard (timeout/close race)
    if error_response:
        invocation->notify_exception(...)
    else:
        invocation->notify(message)  // set_value on promise
    invocations_.erase(entry.id)
```

**Distribution:** `correlationId % num_response_threads` for even load distribution across response threads.

### 4. Buffer Sizing

| Buffer                  | Current                               | New                | Source                                       |
|-------------------------|---------------------------------------|--------------------|----------------------------------------------|
| Application read buffer | 16 KB hardcoded                       | 128 KB             | `socket_options::get_buffer_size_in_bytes()` |
| Application send buffer | None (direct per-message async_write) | 128 KB             | `socket_options::get_buffer_size_in_bytes()` |
| Socket SO_SNDBUF        | 128 KB                                | 128 KB (unchanged) | `socket_options`                             |
| Socket SO_RCVBUF        | 128 KB                                | 128 KB (unchanged) | `socket_options`                             |

The read buffer size change is at `Connection` constructor (currently `read_handler(*this, 16 << 10)`), changed to use
`socket_options::get_buffer_size_in_bytes()`.

### 5. Revised Strand Responsibilities

**Before (strand does everything):**

- Invocation map insert (on write)
- Invocation map lookup (on read)
- Invocation map erase (on deregister)
- Message buffer building
- Socket write (`async_write`)
- Socket read (`async_read_some`)
- Message decoding
- Response processing (`notify()` / `set_value()`)

**After (strand does socket I/O + decode only):**

- Socket write of pre-built batch buffer (`async_write`)
- Socket read into 128 KB buffer (`async_read_some`)
- Message decoding (`ClientMessageBuilder::on_data`)
- Lightweight dispatch (classify message, push to response queue)

**Moved off strand:**

- Invocation map insert → caller thread (global concurrent map)
- Invocation map lookup + erase → response thread (global concurrent map)
- Promise completion / `set_value()` → response thread
- Message serialization → caller thread prepares entry before enqueue

## Complete Data Flow

### Write Path

```
User thread: map->put(key, value)
    |-- call_id_sequence_->next()                    [existing, back pressure]
    |-- generate_new_call_id(connection)              [existing, correlation ID]
    |-- message->set_correlation_id(id)
    |-- invocations_.insert(id, invocation)           [global concurrent_flat_map]
    |-- connection->enqueue_write(new Entry{id, msg}) [lock-free push]
    |-- post(io_context, flush_signal)                [wake IO thread]
    '-- return future from invocation_promise_

IO thread (strand):
    |-- flush_write_queue()
    |   |-- drain write_queue_ (pop all)
    |   |-- copy frames into 128KB send_buffer_
    |   '-- async_write(socket_, send_buffer_)        [ONE syscall for N messages]
    '-- on write complete:
        '-- if more queued -> flush_write_queue()
```

### Read Path

```
IO thread (strand):
    |-- async_read_some into 128KB read_buffer_       [ONE syscall]
    |-- ReadHandler::handle()
    |   '-- ClientMessageBuilder::on_data()           [decode frames]
    |       '-- Connection::handle_client_message(msg)
    |           |-- if BACKUP_EVENT -> notify_backup inline
    |           |-- if IS_EVENT -> listener_service
    |           '-- if RESPONSE -> response_queue_[id % N].push(id, msg)
    '-- do_read() again                               [immediately ready]

Response thread (1 of N):
    |-- entry = queue_.pop()                          [blocks if empty]
    |-- invocations_.visit(entry.id, get invocation)
    |-- invocation->notify(msg) or notify_exception() [set_value on promise]
    |-- invocations_.erase(entry.id)
    '-- loop

User thread:
    '-- future.get() returns                          [.then() on user_executor_]
```

## Error Handling

**Write failure (socket error during async_write):**
For each entry in failed batch: look up in global map by correlation_id, notify exception, erase. Drain remaining write
queue and fail all entries.

**Connection closed:**
Invocation stores `send_connection_` (weak_ptr). On close: scan global map, notify all invocations whose
`send_connection_` matches the closed connection, erase them. Same O(N) scan Java does — bounded by backpressure limit.

**Invocation timeout:**
Timeout handler erases directly from global concurrent map. No strand post needed.

**Race — response arrives after timeout/close:**
Response thread calls `invocations_.visit(id, ...)` — not found — discard. Same as current behavior.

**Race — write queue has entries when connection closes:**
IO thread drains remaining write queue, fails all entries via global map lookup. Then scans global map for remaining
invocations on this connection.

## Synchronization Points

| Point                      | Primitive                                               | Contention                                                 |
|----------------------------|---------------------------------------------------------|------------------------------------------------------------|
| Global invocation map      | `boost::concurrent_flat_map` (sharded internally)       | Low — insert from caller, visit+erase from response thread |
| Write queue per connection | `boost::lockfree::queue<OutboundEntry*>`                | None — lock-free MPSC                                      |
| Response queue per thread  | `std::mutex` + `std::condition_variable` + `std::deque` | Minimal — IO threads push, 1 response thread pops          |

## Configuration

| Property                                 | Default | Description                                       | Status                                  |
|------------------------------------------|---------|---------------------------------------------------|-----------------------------------------|
| `hazelcast.client.io.thread.count`       | 3       | IO threads running io_context                     | Existing (PR #1410)                     |
| `hazelcast.client.response.thread.count` | 2       | Response threads for invocation completion        | Existing property, needs implementation |
| Read/send buffer size                    | 128 KB  | From `socket_options::get_buffer_size_in_bytes()` | Change from 16 KB hardcoded read buffer |

No new user-facing properties. `response.thread.count` already exists but becomes meaningful.

## Preserved Unchanged

- `CallIdSequence` implementations and correlation ID generation
- `ClientMessageBuilder` frame decoding logic
- `invocation_promise_` / `.then()` / `user_executor_` continuation chain
- `socket_options` configuration API
- Smart routing and connection lifecycle
- Heartbeat manager
- Listener service event handling
- Back pressure mechanism

## Implementation Base

All changes build on top of branch `fixIoThreadCount` (PR #1410, hazelcast-cpp-client), which already implements
configurable IO thread count with round-robin connection assignment.

## Thread Count Summary

| Thread type       | Java              | Proposed C++                                   |
|-------------------|-------------------|------------------------------------------------|
| IO input threads  | 3 (NIO selectors) | 3 (Asio io_context runners, shared read+write) |
| IO output threads | 3 (NIO selectors) | (shared with input — Asio proactor is unified) |
| Response threads  | 2                 | 2                                              |
| User executor     | configurable      | Existing `user_executor_`                      |

## Expected Outcome

~3-4x improvement from 103K to 300-400K ops/sec for PUT 4B, based on:

- Removing IO thread blocking on promise completion (~2x)
- Write batching reducing syscalls (~1.5x)
- Larger read buffer reducing read syscalls (~1.2x)
- Reduced strand contention from moving map ops off strand (~1.3x)

These multiply but overlap, so conservative estimate is 3-4x.
