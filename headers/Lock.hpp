#pragma once
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <fcntl.h>    // _O_CREAT, _O_RDWR
#include <io.h>       // _open, _close, _locking
#include <sys/stat.h> // _S_IREAD, _S_IWRITE
#define MAX_PATH 260
#define LK_NBLCK 2
#define LK_UNLCK 0
#else
#include <sys/file.h> // flock
#include <unistd.h>   // close, unlink
#include <fcntl.h>    // open, O_CREAT
#include <errno.h>
#endif

#ifdef _WIN32
static char lock_path[MAX_PATH];
static const char* LOCK_FILE = nullptr;
#else
static const char* LOCK_FILE = "/tmp/myapp.lock";
#endif

static int lock_fd = -1;

inline int acquire_lock()
{
#ifdef _WIN32
	const char* temp_dir = getenv("TEMP");
	if (!temp_dir)
		return -1;

	snprintf(lock_path, MAX_PATH, "%s\\myapp.lock", temp_dir);
	LOCK_FILE = lock_path;

	lock_fd = _open(LOCK_FILE, _O_CREAT | _O_RDWR, _S_IREAD | _S_IWRITE);
	if (lock_fd == -1)
		return -1;

	if (_locking(lock_fd, LK_NBLCK, 1) == -1)
	{
		_close(lock_fd);
		lock_fd = -1;
		return -1;
	}
#else
	// Unix-like file locking
	lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0644);
	if (lock_fd == -1)
		return -1;

	if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1)
	{
		close(lock_fd);
		lock_fd = -1;
		return -1;
	}
#endif
	return 0;
}

inline void release_lock()
{
	if (lock_fd == -1) return;

#ifdef _WIN32
	_locking(lock_fd, LK_UNLCK, 1);
	_close(lock_fd);
#else
	flock(lock_fd, LOCK_UN);
	close(lock_fd);
#endif
	lock_fd = -1;
}
