#include <assert.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <sys/wait.h>

size_t recv(int & err, int rd, char * b, size_t k)
{
  size_t r = 0;
  while (r < k) {
    err = read(rd, b + r, k - r);
    if (err < 0) {
      break;
    }
    r += err;
  }
  return r;
}

int main(int argc, char ** argv)
{
  assert(argc == 2);
  int err = 0;
  int rd = std::atoi(argv[1]);
  assert(rd > 0);
  char msg[256] = {};
  recv(err, rd, msg, 255);
  assert(err > 0);
  err = close(rd);
  assert(!err);
  err = printf("%s", msg);
  assert(err > 0);
}
