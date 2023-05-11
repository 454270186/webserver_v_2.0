#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "buffer.h"
#include "webserver.h"
using namespace std;

int main() {
    int thread_nums = 10;
    WebServer server(thread_nums);
    server.run();
}