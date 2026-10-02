#include <assert.h>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>

char msg[256] = "user data\n";
const char * SHMN = "shm-temp";
constexpr int PAGE_SIZE = 4096;

size_t send(int & err, int wr, const char * b, size_t k)
{
  size_t r = 0;
  while (r < k) {
    err = write(wr, b + r, k - r);
    if (err < 0) {
      break;
    }
    r += err;
  }
  return r;
}

int main()
{
  int pps[2] = {};
  int err = pipe(pps);
  assert(!err);

  int rd = pps[0];
  int wr = pps[1];
  pid_t pid = fork();
  assert(pid >= 0);

  if (!pid) {
    err = close(wr);
    assert(!err);

    char p[100] = {};
    err = sprintf(msg, "%d", rd);
    assert(err > 0);

    execl("child", "clild", msg, NULL);
    assert(0);
  }
  auto fl = O_RDWR | O_CREAT | O_TRUNC;
  auto mode = S_IRUSR | S_IWUSR;
  int shfd = shm_open(SHMN, fl, mode);
  assert(shfd > 0);

  err = ftruncate(shfd, PAGE_SIZE);
  assert(!err);

  int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED;
  auto ptr = (char*)mmap(NULL, PAGE_SIZE, prot, flags, shfd, 0);
  assert(ptr != MAP_FAILED);

  strncpy(ptr, msg, 11);
  err = munmap(ptr, PAGE_SIZE);
  assert(!err);

  err = sprintf(msg, "%d", 10);
  assert(err >= 0);

  send(err, wr, msg, 8);
  assert(err >= 0);

  err = close(wr);
  assert(!err);

  err = close(shfd);
  assert(!err);

  err = waitpid(pid, 0, 0);
  assert(err == pid);

  err = shm_unlink(SHMN);
  assert(!err);
}
