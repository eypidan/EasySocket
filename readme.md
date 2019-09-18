## EasySocket

- The homework in ComputerInternet class.
- Server and client are all implemented in C++.
- Call some POSIX interface.

### Feature

- Multiple thread running, send and receive data  asynchronously

### Build & Run
```shell
cd server
mkdir build
cd build 
cmake ../
make
```

```shell
cd client
mkdir build
cd build 
cmake ../
make
```
#### Build Requirement

- CMake 3.10 versions above
- A C++17-standard-compliant compiler
- Make sure 5750 port is allowed tcp/udp in your host(server)

#### Supported OSes and compilers

- Ubuntu: 16.04 or later: GCC 4.9 or later

