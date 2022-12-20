I tested `transactional_queue` and attached the video about the measurements and behaviour when transaction is rollback.
Consumer and feeder are compiled with `-DCMAKE_BUILD_TYPE=Release` and they run on **Intel i9-10885H** CPU and **Ubuntu 22.04** distro. Compiler is **GCC-11.2.0**.
Both server and clients are on the local machine. Server version is `5.3.0-SNAPSHOT` client version is `5.0.0`.

`feeder` on the video is generated for `examples/transactional-queue-benchmark/feeder.cpp`, it can be built in the build folder with

``` console
cmake --build . --target feeder
```

`feeder` on the video is generated for `examples/transactional-queue-benchmark/consumer.cpp`, it can be built in the build folder with

``` console
cmake --build . --target consumer
```

## Questions ?
- Does rollback put back the item at the back or original place in the queue?

Yes.

- Delays of operations ?

`begin_transaction()` takes 500-600 us.

`poll()` takes 600-700 us.

`commit_transaction()` takes 700-800 us.

`rollback_transaction()` takes 600-700 us.

## Video Explanation (transactional-queue-experiment.mkv)
Left terminal is `feeder`.
Top right terminal is `consumer-1`.
Bottom right terminal is `consumer-2`.

`00:00` - I put 5 items to the queue.

`00:14` - **consumer-1** claims **job-0**.

`00:16` - **consumer-1** processes **job-0**.

`00:19` - **feeder** lists the jobs on the queue.

`00:23` - **consumer-1** claims **job-1**.

`00:24` - **consumer-2** is started.

`00:26` - **consumer-2** claims **job-2**.

`00:28` - **feeder** lists the jobs on the queue.

`00:34` - **consumer-1** fails to process **job-1**.

`00:36` - **consumer-2** fails to process **job-2**.

`00:37` - **feeder** lists the jobs on the queue. Order is preserved.

`00:47` - **consumer-1** claims **job-1** again.

`00:49` - **feeder** lists the jobs on the queue.

`00:50` - **consumer-2** claims **job-2** again.

`00:52` - **feeder** lists the jobs on the queue.

`00:56` - **consumer-1** fails to process **job-1**.

`00:58` - **feeder** lists the jobs on the queue.

`01:02` - **consumer-2** fails to process **job-2**.

`01:04` - **feeder** lists the jobs on the queue. As it is seen, **job-2** is inserted in between **job-1** and **job-3**. So the order is preserved.
