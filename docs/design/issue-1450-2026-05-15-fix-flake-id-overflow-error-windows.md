# Fix: FlakeIdGeneratorApiTest.testSmoke Crash on Windows (std::overflow_error)

**Date**: 2026-05-15  
**Status**: Proposed  
**Repo**: hazelcast-cpp-client  
**Issue**: #1450  
**PR**: TBD  

---

## Problem

`FlakeIdGeneratorApiTest.testSmoke` crashes randomly on `windows-64-(Release, Static, noSSL)` CI
when running the `fixIoThreadCount` branch (PR #1448). The process terminates with a Windows SEH
exception:

```
Exception: E06D7363.?AVoverflow_error@std@@
```

`E06D7363` is the MSVC SEH exception code for any C++ exception thrown via `__CxxThrowException`.  
`?AVoverflow_error@std@@` is MSVC's mangled RTTI name for `std::overflow_error`.

The error occurs **repeatedly** (not a one-off), indicating a race condition that is reliably hit
under the 400,000-ID workload of `testSmoke`.

---

## Root Cause

### 1. Exception used as control-flow signal

`std::overflow_error` is thrown in exactly **one** place in the entire codebase:

**`hazelcast/src/hazelcast/client/proxy.cpp` — `new_id_internal()`**

```cpp
int64_t
flake_id_generator_impl::new_id_internal()
{
    auto b = block_.load();
    if (b) {
        int64_t res = b->next();
        if (res != INT64_MIN) {
            return res;
        }
    }
    throw std::overflow_error("");  // ← control-flow signal: batch exhausted
}
```

It signals "no prefetch batch available" and is intended to be caught immediately in `new_id()`:

```cpp
boost::future<int64_t>
flake_id_generator_impl::new_id()
{
    try {
        return boost::make_ready_future(new_id_internal());
    } catch (std::overflow_error&) {
        return new_id_batch(batch_size_)
          .then(boost::launch::sync,
                [=](boost::future<flake_id_generator_impl::IdBatch> f) {
                    auto newBlock = boost::make_shared<Block>(f.get(), validity_);
                    auto value = newBlock->next();
                    auto b = block_.load();
                    block_.compare_exchange_strong(b, newBlock);
                    return value;
                });
    }
}
```

### 2. The `boost::launch::sync` fallback path

`new_id_batch()` calls `invoke()` which calls `complete_call_id_sequence()`. That function builds
the base future:

```cpp
// spi.cpp — complete_call_id_sequence()
auto& user_executor = execution_service_->get_user_executor();
if (user_executor.closed()) {
    // Fallback: run continuations synchronously in the thread that
    // fires the promise, rather than posting to the user executor.
    return invocation_promise_.get_future().then(
      boost::launch::sync,
      [self, id_seq](boost::future<protocol::ClientMessage> f) {
          id_seq->complete();
          return f.get();
      });
} else {
    return invocation_promise_.get_future().then(
      user_executor, ...);
}
```

The full continuation chain is:

```
invocation_promise_  .then(user_executor OR launch::sync,  id_seq_lambda)  → F1
F1                   .then(launch::sync,                   decode_lambda)  → F2   [new_id_batch]
F2                   .then(launch::sync,                   block_callback) → F3   [new_id]
```

When the `user_executor` is closed (a race during client shutdown while a batch fetch is
in-flight), the `launch::sync` fallback makes the **entire chain execute synchronously in
the thread that fires `invocation_promise_`**. On PR #1448, that thread can be one of the new
IO threads.

### 3. IO threads run without an exception guard

PR #1448 starts multiple IO threads in `ClientConnectionManagerImpl::start()`:

```cpp
// network.cpp:144
io_threads_.emplace_back([raw_ctx]() { raw_ctx->run(); });  // no exception handler
```

`boost::asio::io_context::run()` propagates any uncaught exception from a handler directly to
the caller. If an exception escapes here, the thread function terminates, which on Windows calls
`std::terminate()` via the unhandled SEH filter.

### 4. Why the exception escapes the catch clause

On Windows/MSVC with Release optimizations, Boost.Thread's `boost::launch::sync` mechanism
runs continuations by invoking them directly inside the thread that fires the parent promise.
Internally this looks approximately like:

```cpp
// Conceptual Boost.Thread internals for launch::sync:
try {
    promise.set_value(continuation_callback(std::move(parent_future)));
} catch (...) {
    promise.set_exception(boost::current_exception());
}
```

On Linux/GCC this reliably catches and re-stores the exception. On Windows/MSVC with Release
optimizations and SEH-based exception handling, `boost::current_exception()` interacts with the
active SEH exception context. When `std::overflow_error` was active in a parent `catch` block at
the time the async chain was set up, the SEH exception context can "bleed" into the continuation
execution environment, causing `boost::current_exception()` to re-capture the original
`overflow_error` instead of the continuation's exception — and re-throw it in a context where
there is no corresponding `catch (std::overflow_error&)`.

This explains why the developer's earlier patches adding `catch (std::exception& ex)` and
`catch (...)` to `ClientInvocation::set_exception()` (commits `809bb5dc9`, `09ec6a5a8`) did
not fully resolve the issue: those catches guard one path, but the exception escapes via
the `raw_ctx->run()` / IO-thread path as well.

### 5. Why this is a regression vs. master

`master` uses a single IO thread; PR #1448 introduces multiple IO contexts with unguarded
`run()` calls. The increased parallelism raises the probability of the shutdown-race that
triggers the `launch::sync` fallback, and the new IO threads are the crash site.

### 6. Frequency

`testSmoke` generates ~4,000 batch-exhaustion events per run (400,000 IDs ÷ default prefetch
batch size of 100). Each event throws `overflow_error`. With many throws per run, even a
low-probability escape becomes a near-certainty on Windows MSVC Release builds.

---

## Solution

Two complementary changes are made.

### Change 1: Eliminate `std::overflow_error` as control flow (primary fix)

**File**: `hazelcast/src/hazelcast/client/proxy.cpp`

`Block::next()` already uses `INT64_MIN` as a sentinel to signal "exhausted or expired." We
adopt the same pattern in `new_id_internal()` / `new_id()`, removing the exception entirely.

#### `new_id_internal()` — return sentinel instead of throwing

```cpp
// BEFORE
int64_t
flake_id_generator_impl::new_id_internal()
{
    auto b = block_.load();
    if (b) {
        int64_t res = b->next();
        if (res != INT64_MIN) {
            return res;
        }
    }
    throw std::overflow_error("");
}

// AFTER
int64_t
flake_id_generator_impl::new_id_internal()
{
    auto b = block_.load();
    if (b) {
        int64_t res = b->next();
        if (res != INT64_MIN) {
            return res;
        }
    }
    return INT64_MIN;
}
```

#### `new_id()` — branch on sentinel instead of catching exception

```cpp
// BEFORE
boost::future<int64_t>
flake_id_generator_impl::new_id()
{
    try {
        return boost::make_ready_future(new_id_internal());
    } catch (std::overflow_error&) {
        return new_id_batch(batch_size_)
          .then(boost::launch::sync,
                [=](boost::future<flake_id_generator_impl::IdBatch> f) {
                    auto newBlock = boost::make_shared<Block>(f.get(), validity_);
                    auto value = newBlock->next();
                    auto b = block_.load();
                    block_.compare_exchange_strong(b, newBlock);
                    return value;
                });
    }
}

// AFTER
boost::future<int64_t>
flake_id_generator_impl::new_id()
{
    auto res = new_id_internal();
    if (res != INT64_MIN) {
        return boost::make_ready_future(res);
    }
    return new_id_batch(batch_size_)
      .then(boost::launch::sync,
            [=](boost::future<flake_id_generator_impl::IdBatch> f) {
                auto newBlock = boost::make_shared<Block>(f.get(), validity_);
                auto value = newBlock->next();
                auto b = block_.load();
                block_.compare_exchange_strong(b, newBlock);
                return value;
            });
}
```

After this change, `grep -r "overflow_error" hazelcast/src hazelcast/include` returns zero
results (aside from the unrelated Hazelcast-protocol `stack_overflow_error` exception class).
The crash is structurally impossible.

### Change 2: Guard IO thread loops against uncaught exceptions (defensive fix)

**File**: `hazelcast/src/hazelcast/client/network.cpp`

Wrap `raw_ctx->run()` with a try-catch so that any future handler misbehaviour cannot silently
terminate an IO thread:

```cpp
// BEFORE
io_threads_.emplace_back([raw_ctx]() { raw_ctx->run(); });

// AFTER
io_threads_.emplace_back([raw_ctx, this]() {
    try {
        raw_ctx->run();
    } catch (const std::exception& e) {
        HZ_LOG(logger_,
               severe,
               boost::str(boost::format(
                            "IO thread terminated with exception: %1%") %
                          e.what()));
    } catch (...) {
        HZ_LOG(logger_, severe,
               "IO thread terminated with unknown exception");
    }
});
```

This ensures that even if a future exception escapes through the Boost.Thread/Asio machinery,
the IO thread logs it and exits cleanly rather than calling `std::terminate()`.

---

## Alternatives Considered

| Alternative | Why rejected |
|---|---|
| Add broader `catch(...)` in more places throughout the invocation path | Whack-a-mole; does not fix the root cause. Developer already tried this in `set_exception()` and crashes persisted. |
| Set a `boost::asio::thread_pool`-level exception handler | Asio thread_pool does not expose a per-exception hook; would require forking Boost. |
| Replace `boost::launch::sync` with `launch::deferred` | `deferred` runs in the `.get()` caller's thread (test thread), which changes scheduling behaviour and could introduce different issues. Not a safe drop-in replacement. |

---

## Testing

1. Build `windows-64-(Release, Static, noSSL)` (MSVC x64 Release Static).
2. Run:
   ```
   client_test.exe --gtest_filter="FlakeIdGeneratorApiTest.*" --gtest_repeat=50
   ```
3. Confirm zero `E06D7363` / `overflow_error` events across all repetitions.
4. Run the full test suite to confirm no regressions.
5. Verify:
   ```bash
   grep -r "overflow_error" hazelcast/src hazelcast/include
   ```
   should match only `protocol_exceptions.h` (`stack_overflow_error`) — zero occurrences in
   `proxy.cpp`.

### Local stress-reproducer (before fix)

```cpp
client_flake_id_generator_config cfg("gen");
cfg.set_prefetch_count(1);   // forces a batch fetch on every single new_id() call
```

Adding this config to the test fixture and running on Windows Release will trigger the crash
within a few hundred iterations.
