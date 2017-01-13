# Hazelcast C++ Map Interceptor Example

This example demonstrates the usage of map interceptor on a C++ client.

## Running the Example

To run this example you need to have a running Hazelcast server which includes definition for Identified Data Serializable object called Map Interceptor. To start such Hazelcast instance, you can run the interceptorServer.sh under java directory of the repository. To build the examples, you can refer to readme file of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

To run mapinterceptor(main.cpp), execute the command below,
```
sh ./run-interceptor.sh {MYPATH}
```
