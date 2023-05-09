#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "buffer.h"
#include "webserver.h"
using namespace std;

int main() {
    WebServer server(10);
    server.run();
}