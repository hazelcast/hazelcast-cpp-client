# Hazelcast C++ Indexed Query Example

This example demonstrates the usage of query with indexed entry on a C++ client.

## Running the Example

To run this example you need to have a running Hazelcast server which includes definition for Identified Data Serializable object called Person. To start such Hazelcast instance, you can run the indexqueryServer.sh under java directory of the repository. To build the examples, you can refer to readme file of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

To run queryindexedentry(main.cpp), execute the command below,
```
sh ./run-index.sh {MYPATH}
```
