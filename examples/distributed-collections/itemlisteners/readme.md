# Hazelcast C++ Client Item Listener Example

This example demonstrates the usage of Item Listener on a C++ client.

## Running the Example

To run examples you need to have a running Hazelcast cluster. To start a Hazelcast instance, you can run the basicServer.sh under java directory of the repository. To build the examples, you can refer to readme file of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

Firstly, execute the itemListener (ItemListener.cpp) which registers an item listener to an Hazelcast Queue and waits for the events.

To run itemListener, execute the command below,
```
sh ./run-listener.sh {MYPATH}
```

After that, while itemListener is waiting, execute the collectionchanger (CollectionChanger.cpp) which makes queue operations.
To run collectionchanger, execute the command below,
```
sh ./run-changer.sh {MYPATH}
```

In the end, you are supposed to see recorded events, 2 addition and 1 remove event, on mapentrylistener's console log.
```
Item added:foo
Item added:bar
Item removed:foo
Item removed:bar
Received 2 items addition and 2 items removal events.
```
