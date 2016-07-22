# Hazelcast C++ Client ISemaphore Example

This example demonstrates the usage of ISemaphore on a C++ client.

## Running the Example

To run example you need to have a running Hazelcast cluster.

Since the ISemaphore is initialized with 0 permit, there is a need for a Hazelcast instance which initializes a semaphore with 1 permits. You can start such instance by running semaphoreServer.sh under java directory of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

To run semaphore, execute the command below,
```
sh ./run-semaphore.sh {MYPATH}
```
