# Hazelcast C++ Client Entry Processor Example

This example demonstrates the usage of Entry Processor on a C++ client. It also contains the version which does not use entry processor to manipulate the objects.

## Running the Example

To run examples you need to have a running Hazelcast cluster.
* For the entry processor version, you need a server which includes definition for Identified Data Serializable object called EmployeeRaiseEntryProcessor. For such instance, you can run processorServer.sh under java directory of the repository.
* For the no processor version, you can run the basicServer.sh under java directory of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

To run mapentryprocessor(main.cpp), execute the command below,
```
sh ./run-entryprocessor.sh {MYPATH}
```

To run modifywithoutentryprocessor(ModifyWithNoEntryProcessor.cpp), execute the command below,
```
sh ./run-noprocessor.sh {MYPATH}
```
