#include <assert.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

const char * SHMN = "/shm-temp";
constexpr size_t MAX_NUMBER_LEN = 100;

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
    err = sprintf(p, "%d", rd);
    assert(err > 0);

    execl("child", "child", p, NULL);
    assert(0);
  }

  err = close(rd);
  assert(!err);

  std::string text;
  std::getline(std::cin, text);

  size_t size = text.length() + 1;
  const char * msg = text.c_str();

  auto fl = O_RDWR | O_CREAT | O_TRUNC;
  auto mode = S_IRUSR | S_IWUSR;
  int shfd = shm_open(SHMN, fl, mode);
  assert(shfd >= 0);

  err = ftruncate(shfd, size);
  assert(!err);

  int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED;
  auto ptr = (char*)mmap(NULL, size, prot, flags, shfd, 0);
  assert(ptr != MAP_FAILED);

  memcpy(ptr, msg, size);
  err = munmap(ptr, size);
  assert(!err);

  char p[MAX_NUMBER_LEN] = {};
  err = sprintf(p, "%zu", size);
  assert(err >= 0);

  send(err, wr, p, MAX_NUMBER_LEN);
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
