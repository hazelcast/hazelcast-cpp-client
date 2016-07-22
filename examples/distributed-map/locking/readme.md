# Hazelcast C++ Client Locking Example

This example demonstrates the usage of Lock on a C++ client and consist of 4 different examples.

## Running the Example

To run this example you need to have a running Hazelcast server which includes definition for Identified Data Serializable object called Value. To start such Hazelcast instance, you can run the lockingServer.sh under java directory of the repository. To build the examples, you can refer to readme file of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

To run AbaProtectedOptimisticUpdate, execute the command below,
```
sh ./run-abaprotected.sh {MYPATH}
```

To run OptimisticUpdate, execute the command below,
```
sh ./run-optimistic.sh {MYPATH}
```

To run PessimisticUpdate, execute the command below,
```
sh ./run-pessimistic.sh {MYPATH}
```

To run RacyUpdate, execute the command below,
```
sh ./run-racy.sh {MYPATH}
```
