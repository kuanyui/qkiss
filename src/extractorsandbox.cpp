/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2026 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "extractorsandbox.h"
#include "logging.h"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>

#ifdef Q_OS_LINUX
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/landlock.h>
#include <linux/seccomp.h>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace {


// ======================================================
// Notes
// ======================================================
// - All code within this anonymous namespace runs in the child process, **between fork() and exec()**.
// - The parent also calls landlockCreateRuleset(), but only to log the Landlock ABI version.
// - In the child, only async-signal-safe functions may be called: no memory allocation, no Qt. So this code uses system calls and C strings.


// ======================================================
// Landlock System Call Wrapper Functions
// ======================================================

// glibc has no wrappers for Landlock, so wrap them by ourselves.

/// Create empty ruleset and return a fd of it.
int landlockCreateRuleset(const landlock_ruleset_attr *attributes, size_t size, __u32 flags)
{
    return static_cast<int>(syscall(__NR_landlock_create_ruleset, attributes, size, flags));
}

/// Yeah... syscall `__NR_landlock_create_ruleset` is also used to get Landlock ABI version...
/// -1 means not support.
int landlockGetAbiVersion()
{
    return static_cast<int>(syscall(__NR_landlock_create_ruleset, nullptr, 0, LANDLOCK_CREATE_RULESET_VERSION));
}

/// Add rule into ruleset (ex: `/usr` is RO)
int landlockAddPathRule(int rulesetFd, const landlock_path_beneath_attr *rule)
{
    return static_cast<int>(syscall(__NR_landlock_add_rule, rulesetFd, LANDLOCK_RULE_PATH_BENEATH, rule, 0));
}

/// Apply ruleset to self (and cannot be removed by itself anymore.)
int landlockRestrictSelf(int rulesetFd)
{
    return static_cast<int>(syscall(__NR_landlock_restrict_self, rulesetFd, 0));
}

// ======================================================
// Landlock Contants
// ======================================================

constexpr __u64 FS_READ =
        LANDLOCK_ACCESS_FS_READ_FILE |
        LANDLOCK_ACCESS_FS_READ_DIR;
constexpr __u64 FS_EXECUTE =
        LANDLOCK_ACCESS_FS_EXECUTE;
constexpr __u64 FS_WRITE =
        LANDLOCK_ACCESS_FS_WRITE_FILE |
        LANDLOCK_ACCESS_FS_REMOVE_DIR |
        LANDLOCK_ACCESS_FS_REMOVE_FILE |
        LANDLOCK_ACCESS_FS_MAKE_DIR |
        LANDLOCK_ACCESS_FS_MAKE_REG |
        LANDLOCK_ACCESS_FS_MAKE_SYM |
        LANDLOCK_ACCESS_FS_REFER |
        LANDLOCK_ACCESS_FS_TRUNCATE;

/// The only rights that a rule on a file (**NOT** a folder!!!) may have.
constexpr __u64 FILE_RIGHTS =
        LANDLOCK_ACCESS_FS_EXECUTE |
        LANDLOCK_ACCESS_FS_WRITE_FILE |
        LANDLOCK_ACCESS_FS_READ_FILE |
        LANDLOCK_ACCESS_FS_TRUNCATE |
        LANDLOCK_ACCESS_FS_IOCTL_DEV;

struct PathRule {
    const char *path;
    __u64 access;
};

/// The necessary paths that LZH extractor may use.
constexpr PathRule SYSTEM_PATH_RULES[] = {
    { "/usr", FS_READ | FS_EXECUTE },
    { "/bin", FS_READ | FS_EXECUTE },
    { "/sbin", FS_READ | FS_EXECUTE },
    { "/lib", FS_READ | FS_EXECUTE },
    { "/lib32", FS_READ | FS_EXECUTE },
    { "/lib64", FS_READ | FS_EXECUTE },
    { "/etc", FS_READ },
    { "/proc/self", FS_READ },               // Extractor process itself
    { "/sys/devices/system/cpu", FS_READ },  // The number of CPUs, for glibc.
    { "/dev/null", FS_READ | FS_WRITE },
    { "/dev/random", FS_READ },
    { "/dev/urandom", FS_READ },
};

/// - Get our-defined list of `LANDLOCK_ACCESS_FS_*`, which will be blocked by default.
///   - `landlock_ruleset_attr` contains fields `handled_access_fs`, `handled_access_net`, `scoped`. This function only returns `handled_access_fs`.
/// - If set this into `landlock_ruleset_attr.handled_access_fs`, **ALL paths in this system** will be denied by default (blacklist mode),
///   you need to add exceptions into whitelist via `landlock_path_beneath_attr.allowed_access` and `__NR_landlock_add_rule`
/// @return `fs_access` (`landlock_ruleset_attr.handled_access_fs`)
__u64 getDefaultBlockedLandlockAccesFs(int abi)
{
    __u64 access =
            LANDLOCK_ACCESS_FS_EXECUTE |
            LANDLOCK_ACCESS_FS_WRITE_FILE |
            LANDLOCK_ACCESS_FS_READ_FILE |
            LANDLOCK_ACCESS_FS_READ_DIR |
            LANDLOCK_ACCESS_FS_REMOVE_DIR |
            LANDLOCK_ACCESS_FS_REMOVE_FILE |
            LANDLOCK_ACCESS_FS_MAKE_CHAR |
            LANDLOCK_ACCESS_FS_MAKE_DIR |
            LANDLOCK_ACCESS_FS_MAKE_REG |
            LANDLOCK_ACCESS_FS_MAKE_SOCK |
            LANDLOCK_ACCESS_FS_MAKE_FIFO |
            LANDLOCK_ACCESS_FS_MAKE_BLOCK |
            LANDLOCK_ACCESS_FS_MAKE_SYM;
    if (abi >= 2) { access |= LANDLOCK_ACCESS_FS_REFER; }
    if (abi >= 3) { access |= LANDLOCK_ACCESS_FS_TRUNCATE; }
    if (abi >= 5) { access |= LANDLOCK_ACCESS_FS_IOCTL_DEV; }
    return access;
}

/// No TCP bind or connect.
/// NOTE: `seccomp` will still be used to restrict the `socket()` syscall, which can restrict UDP and UNIX socket
/// @return `net_access` (`landlock_ruleset_attr.handled_access_net`)
__u64 getDefaultBlockedLandlockAccessNet(int abi)
{
    if (abi < 4) { return 0; }
    return LANDLOCK_ACCESS_NET_BIND_TCP | LANDLOCK_ACCESS_NET_CONNECT_TCP;
}

/// Signals and abstract UNIX sockets only within the sandbox.
/// @return `scope` (`landlock_ruleset_attr.scoped`)
__u64 getLandlockScoped(int abi)
{
    if (abi < 6) { return 0; }
    return LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET | LANDLOCK_SCOPE_SIGNAL;
}

// ======================================================
// Landlock Logics
// ======================================================

/// Add allow (`access`) rule to ruleset.
/// Grants `access` beneath the folder `path`, or on `path` itself if it is a file.
/// @returns false on error. A path that does not exist is skipped without raising error.
bool allowPath(int rulesetFd, const char *path, __u64 accessFlags)
{
    // (Remind: By default, `open()` follows symlink.)
    // O_PATH: locate the path only (merely ensure it's existed) without even read permission.
    // O_CLOEXEC: close-on-exec. When process calls `exec()`, the fd will be closed by kernel automatically.
    const int fd = open(path, O_PATH | O_CLOEXEC);
    if (fd < 0) { return errno == ENOENT; }  // If the path doesn't exist, return true.
    struct stat status;
    bool ok = fstat(fd, &status) == 0;
    if (ok) {
        landlock_path_beneath_attr rule = {};
        rule.parent_fd = fd;
        rule.allowed_access = S_ISDIR(status.st_mode) ? accessFlags : accessFlags & FILE_RIGHTS;
        ok = rule.allowed_access == 0 || landlockAddPathRule(rulesetFd, &rule) == 0;
    }
    close(fd);
    return ok;
}

/// Limits the file system with Landlock.
/// This function will:
/// 1. Check availability of Landlock.
/// 2. Create a `landlock_ruleset_attr`, then fill the following rules into this structure:
///       (i) Deny all FS & Net access by default
///      (ii) Add pre-defined, limited-accessible system paths (ex: `/usr` is RO, `/dev/null` is RW)
///     (iii) Allow some dynamic paths which are determined during runtime. (ex: `lha` path, lzh file path)
/// 3. Create a ruleset and get its fd, pass the `landlock_ruleset_attr` into the fd.
/// 4. Apply the ruleset on the current process. The process is restricted from now on.
/// @returns false on error. (A kernel without Landlock is not an error.)
bool restrictFileSystemViaLandlock(const char *program, const char *archive, const char *writableDir)
{
    const int abiVersion = landlockGetAbiVersion();
    if (abiVersion < 0) {
        return errno == ENOSYS || errno == EOPNOTSUPP;  // Not built into this kernel, or disabled at boot.
    }
    landlock_ruleset_attr attributes = {};
    // (i) Deny all FS & Net access by default
    attributes.handled_access_fs = getDefaultBlockedLandlockAccesFs(abiVersion);   // deny all fs access in this system
    attributes.handled_access_net = getDefaultBlockedLandlockAccessNet(abiVersion);
    attributes.scoped = getLandlockScoped(abiVersion);

    const int rulesetFd = landlockCreateRuleset(&attributes, sizeof(attributes), 0);
    if (rulesetFd < 0) { return false; }

    const __u64 allowedFsFlags = attributes.handled_access_fs;
    bool ok = true;
    // (ii) Add pre-defined, limited-accessible system paths (ex: `/usr` is RO, `/dev/null` is RW)
    for (const PathRule &pathRule : SYSTEM_PATH_RULES) {
        ok = ok && allowPath(rulesetFd, pathRule.path, pathRule.access & allowedFsFlags);
    }
    // (iii) Allow some dynamic paths which are determined during runtime. (ex: `lha` path, lzh file path)
    ok = ok && (program == nullptr || allowPath(rulesetFd, program, (FS_READ | FS_EXECUTE) & allowedFsFlags));
    ok = ok && allowPath(rulesetFd, archive, FS_READ & allowedFsFlags);
    ok = ok && (writableDir == nullptr || allowPath(rulesetFd, writableDir, (FS_READ | FS_WRITE) & allowedFsFlags));
    ok = ok && landlockRestrictSelf(rulesetFd) == 0;
    close(rulesetFd);
    return ok;
}

// ======================================================
// `seccomp` Constants
// ======================================================

// The system call numbers in the filter are only valid for the architecture they were compiled for.
#if defined(__x86_64__)
#define SANDBOX_SECCOMP_ARCH AUDIT_ARCH_X86_64
#elif defined(__aarch64__)
#define SANDBOX_SECCOMP_ARCH AUDIT_ARCH_AARCH64
#endif

#ifdef SANDBOX_SECCOMP_ARCH
// Makes the system call `name` fail with `error`.
#define DENY(name, error) \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_##name, 0, 1), \
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | (error))

constexpr __u32 NEW_NAMESPACES =
        CLONE_NEWNS |
        CLONE_NEWCGROUP |
        CLONE_NEWUTS |
        CLONE_NEWIPC |
        CLONE_NEWUSER |
        CLONE_NEWPID |
        CLONE_NEWNET;

constexpr sock_filter SECCOMP_FILTER[] = {
    // Kill the process on a system call of another architecture: the numbers below mean other system calls there.
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, arch)),
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SANDBOX_SECCOMP_ARCH, 1, 0),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, nr)),
#ifdef __x86_64__
    // x32 system calls come as AUDIT_ARCH_X86_64 too, with this bit set in their numbers.
    BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, __X32_SYSCALL_BIT, 0, 1),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
#endif
    // Network, and local sockets such as D-Bus, X11 and Wayland.
    DENY(socket, EPERM),
    DENY(socketpair, EPERM),
    // Namespaces. A filter cannot read the flags of clone3(), which are in memory: ENOSYS makes glibc use clone().
    DENY(clone3, ENOSYS),
    DENY(unshare, EPERM),
    DENY(setns, EPERM),
    // Other processes: their memory and their file descriptors.
    DENY(ptrace, EPERM),
    DENY(process_vm_readv, EPERM),
    DENY(process_vm_writev, EPERM),
    DENY(pidfd_getfd, EPERM),
    // Kernel attack surfaces that command-line tools do not need.
    DENY(io_uring_setup, EPERM),
    DENY(io_uring_enter, EPERM),
    DENY(io_uring_register, EPERM),
    DENY(bpf, EPERM),
    DENY(perf_event_open, EPERM),
    DENY(userfaultfd, EPERM),
    DENY(keyctl, EPERM),
    DENY(add_key, EPERM),
    DENY(request_key, EPERM),
    DENY(mount, EPERM),
    DENY(umount2, EPERM),
    DENY(pivot_root, EPERM),
    DENY(chroot, EPERM),
    DENY(fsopen, EPERM),
    DENY(fsconfig, EPERM),
    DENY(fsmount, EPERM),
    DENY(fspick, EPERM),
    DENY(move_mount, EPERM),
    DENY(open_tree, EPERM),
    DENY(mount_setattr, EPERM),
    // Running a program from memory, which Landlock cannot check.
    DENY(memfd_create, EPERM),
    DENY(execveat, EPERM),
    // clone() without new namespaces. Its flags are the first argument (the low 32 bits come first on little-endian).
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clone, 0, 3),
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, args)),
    BPF_JUMP(BPF_JMP | BPF_JSET | BPF_K, NEW_NAMESPACES, 0, 1),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | EPERM),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
};
#undef DENY


// ======================================================
// `seccomp` Logics
// ======================================================


/// Limits the system calls with seccomp. Returns false on error.
bool restrictSystemCallsViaSeccomp()
{
    const sock_fprog program = {
        static_cast<unsigned short>(sizeof(SECCOMP_FILTER) / sizeof(SECCOMP_FILTER[0])),
        const_cast<sock_filter *>(SECCOMP_FILTER),
    };
    return prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &program) == 0;
}
#else
/// No filter for this architecture yet.
bool restrictSystemCallsViaSeccomp()
{
    return true;
}
#endif

// ======================================================
// Main
// ======================================================


void writeError(const char *text)
{
    const ssize_t written = write(STDERR_FILENO, text, strlen(text));
    Q_UNUSED(written);  // Nothing to do if it fails: the process ends anyway.
}

[[noreturn]] void failSandbox(const char *step)
{
    writeError("QKiSS: cannot sandbox the LZH extractor: ");
    writeError(step);
    writeError("\n");
    _exit(127);
}

/// Runs in the child process, just before exec().
void restrictThisProcess(const char *program, const char *archive, const char *writableDir)
{
    // Required by Landlock and by seccomp without root. The extractor can never gain privileges, e.g. from setuid.
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) { failSandbox("no_new_privs"); }
    if (!restrictFileSystemViaLandlock(program, archive, writableDir)) { failSandbox("Landlock"); }
    if (!restrictSystemCallsViaSeccomp()) { failSandbox("seccomp"); }
}

} // namespace
#endif // Q_OS_LINUX


// ======================================================
// Public APIs
// ======================================================

void sandboxExtractorProcess(QProcess *process, const QString &program, const QString &archiveFilePath, const QString &writableDirPath)
{
#ifdef Q_OS_LINUX
    // Converted here, in the parent process, because the child must not allocate memory.
    const QByteArray programPath = QFile::encodeName(QStandardPaths::findExecutable(program));
    const QByteArray archive = QFile::encodeName(archiveFilePath);
    const QByteArray writableDir = QFile::encodeName(writableDirPath);
    process->setChildProcessModifier([programPath, archive, writableDir]() {
        restrictThisProcess(programPath.isEmpty() ? nullptr : programPath.constData(), archive.constData(),
                            writableDir.isEmpty() ? nullptr : writableDir.constData());
    });
    qkDebug << "Sandbox for" << program << "with Landlock ABI" << landlockGetAbiVersion() << "(Note: -1 means no Landlock)";
#else
    Q_UNUSED(process);
    Q_UNUSED(program);
    Q_UNUSED(archiveFilePath);
    Q_UNUSED(writableDirPath);
#endif
}
