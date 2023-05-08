#include <iostream>
#include "buffer.h"
using namespace std;

int main() {
    Buffer buf(10);
    buf.append("qwertyuiopasdfghjkl");
    buf.print_buf();
}