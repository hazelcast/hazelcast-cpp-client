#### Contributing
If you would like to contribute to hazelcast-cpp-client;
- Fork the original repo
- Commit your contributions in a new branch
- Send a Pull Request

For detailed instructions for our git process: [Developing with Git](https://hazelcast.atlassian.net/wiki/display/COM/Developing+with+Git)

In order to merge your pull request, we need the contributor to sign [Hazelcast Contributor Agreement](https://hazelcast.atlassian.net/wiki/display/COM/Hazelcast+Contributor+Agreement). You do not have to wait for approval for doing this step.

Please make sure that your code passes `testLinux.sh` for Linux/Mac and `testWindows.bat` for MS Windows. The script executes builds and runs the unit tests for all bit and library versions. (Alternatively, you can also use `testLinuxSingleCase.sh` or `testWindowsSingleCase.sh` for testing with a specific bit aversion and library type. 
