# Hazelcast C++ Client Entry Listener Example

This example demonstrates the usage of Entry Listener on a C++ client.

## Running the Example

To run examples you need to have a running Hazelcast cluster. To start a Hazelcast instance, you can run the basicServer.sh under java directory of the repository. To build the examples, you can refer to readme file of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

Firstly, execute the mapentrylistener (main.cpp) which registers an entry listener to an IMap and waits for the events.

To run mapentrylistener, execute the command below,
```
sh ./run-entrylistener.sh {MYPATH}
```

After that, while mapentrylistener is waiting, execute the modifymap (ModifyMap.cpp) which makes map operations.
To run modifymap, execute the command below,
```
sh ./run-modifier.sh {MYPATH}
```

In the end, you are supposed to see recorded events, 2 addition and 1 remove event, on mapentrylistener's console log.
```
[entryAdded] EntryEvent{entryEventType=11, member=Member[Address[127.0.0.1:5701]], name='somemap', key=1, value=100
[entryAdded] EntryEvent{entryEventType=11, member=Member[Address[127.0.0.1:5701]], name='somemap', key=2, value=200
[entryRemoved] EntryEvent{entryEventType=22, member=Member[Address[127.0.0.1:5701]], name='somemap', key=1
```
