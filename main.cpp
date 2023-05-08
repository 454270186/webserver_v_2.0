#include <iostream>
#include "buffer.h"
using namespace std;

int main() {
    Buffer buf(10);
    buf.append("qwertyuiopasdfghjkl");
    buf.print_buf();
    buf.retrieve_all();
    buf.print_buf();
    printf("%ld\n", buf.readable_size());
}