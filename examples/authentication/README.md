# username_password_authentication

- Start server:
    ```bash
    java -Dhazelcast.config=./examples/authentication/hazelcast-username-password.xml -jar hazelcast-<VERSION>.jar
    ```
- Run client test:
    ```bash
    ./build/examples/authentication/username_password_authentication 
  ```

# token_authentication

- Start server:
    ```bash
    java -Dhazelcast.config=./examples/authentication/hazelcast-token-credentials.xml -jar hazelcast-<VERSION>.jar
    ```
- Run client test:
    ```bash
    ./build/examples/authentication/token_authentication 
  ```