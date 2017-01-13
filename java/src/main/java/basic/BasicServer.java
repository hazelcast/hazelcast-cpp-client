package basic;

import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;

public class BasicServer {
    public static void main(String args[]) {
        HazelcastInstance instance = Hazelcast.newHazelcastInstance();
    }
}
