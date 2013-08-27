/*
 * Copyright (c) 2008-2013, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * User: sancar
 * Date: 8/26/13
 * Time: 1:51 PM
 */
public class ClientTCPIPListener {
    static final int OK = 5678;
    static final int END = 1;
    static final int START = 2;
    static final int SHUTDOWN = 3;
    static final int SHUTDOWN_ALL = 4;

    public static void main(String args[]) throws IOException {

        final ServerSocket welcomeSocket = new ServerSocket(6000);
        final Map<Integer, HazelcastInstance> map = new HashMap<Integer, HazelcastInstance>();
        final AtomicInteger atomicInteger = new AtomicInteger(0);
        while (true) {
            final Socket socket = welcomeSocket.accept();
            final ExecutorService executorService = Executors.newSingleThreadExecutor();
            executorService.submit(new Runnable() {
                public void run() {
                    try {
                        final DataInputStream dataInputStream = new DataInputStream(socket.getInputStream());
                        final DataOutputStream dataOutputStream = new DataOutputStream(socket.getOutputStream());
                        while (true) {
                            final int command = dataInputStream.readInt();
                            switch (command) {
                                case START:
                                    final int id = atomicInteger.incrementAndGet();
                                    System.out.println(id);
                                    map.put(id, Hazelcast.newHazelcastInstance());
                                    dataOutputStream.writeInt(id);
                                    break;
                                case SHUTDOWN:
                                    final int id2 = dataInputStream.readInt();
                                    final HazelcastInstance instance = map.get(id2);
                                    if (instance == null) {
                                        dataOutputStream.writeInt(OK);
                                        continue;
                                    }
                                    instance.getLifecycleService().shutdown();
                                    dataOutputStream.writeInt(OK);
                                    break;
                                case SHUTDOWN_ALL:
                                    Hazelcast.shutdownAll();
                                    map.clear();
                                    dataOutputStream.writeInt(OK);
                                    break;
                                case END:
                                    System.exit(0);
                                    break;
                            }
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            });
        }


    }
}
