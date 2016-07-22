package primitives;

import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.ISemaphore;

public class SemaphoreServer {

    public static void main(String[] args) {
        HazelcastInstance hz = Hazelcast.newHazelcastInstance();
        ISemaphore semaphore = hz.getSemaphore("semaphore");
        semaphore.init(1);
    }
}
