# Hazelcast C++ Client ITopic Example

This example demonstrates the usage of ITopic on a C++ client.

## Running the Example

To run examples you need to have a running Hazelcast cluster. To start a Hazelcast instance, you can run the basicServer.sh under java directory of the repository. To build the examples, you can refer to readme file of the repository.

You need the PATH of cpp build directory, where the executable files are located. ({MYPATH})

Firstly, execute the Subscriber which registers an message listener to an ITopic and waits for the messages.

To run Subscriber, execute the command below,
```
sh ./run-subscriber.sh {MYPATH}
```

After that, while Subscriber is waiting, execute the Publisher which publishes a message.
To run Publisher, execute the command below,
```
sh ./run-publisher.sh {MYPATH}
```

In the end, you are supposed to see recorded message:
```
[MyTopicListener::onMessage] Message received:
```
