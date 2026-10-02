#include <assert.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

const char * SHMN = "shm-temp";

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

  char msg[100] = {};
  recv(err, rd, msg, 8);
  assert(err > 0);

  err = close(rd);
  assert(!err);

  int dt = std::atoi(msg);
  assert(dt > 0);

  auto fl = O_RDONLY;
  auto mode = S_IRUSR | S_IWUSR;
  int shfd = shm_open(SHMN, fl, mode);
  assert(shfd > 0);

  int prot = PROT_READ;
  int flags = MAP_SHARED;
  auto ptr = (char*)mmap(NULL, dt, prot, flags, shfd, 0);
  assert(ptr != MAP_FAILED);

  err = printf("%s", ptr);
  assert(err == dt);

  err = munmap(ptr, dt);
  assert(!err);

  err = close(shfd);
  assert(!err);
}
