#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define NAME_LEN       16
#define FILE_LEN       32
#define PID_LEN        6
#define REBOOT_DEV     "/dev/imx6q_reboot"
#define REBOOT_MAGIC   'R'
#define REBOOT_IOC_RESET _IO(REBOOT_MAGIC, 0x01)

void setSched(int policy, int priority)
{
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    if (sched_getparam(0, &param) < 0) {
        printf("sched_getparam error: %m\n");
    } else {
        param.sched_priority = priority;
        if (sched_setscheduler(0, policy, &param) < 0) {
            printf("sched_setscheduler failed: %m\n");
        }
    }
}

int main(int argc, char **argv)
{
    int fd, ret;
    ssize_t num;
    char filepath[FILE_LEN];
    char pid[PID_LEN];
    char name[NAME_LEN * 2];
    char *name_str;
    pid_t ppid;

    setSched(SCHED_FIFO, 98);

    ppid = getppid();
    snprintf(pid, sizeof(pid), "%d", ppid);

    memset(filepath, 0, FILE_LEN);
    strncat(filepath, "/proc/", FILE_LEN - 1);
    strncat(filepath, pid, FILE_LEN - 1);
    strncat(filepath, "/stat", FILE_LEN - 1);

    memset(name, 0, sizeof(name));
    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        printf("open file(%s) error: %m\n", filepath);
    } else {
        num = read(fd, name, sizeof(name) - 1);
        if (num <= 0) {
            printf("read file(%s) error: %m\n", filepath);
        }
        close(fd);
    }

    name_str = strchr(name, '(');
    if (name_str != NULL) {
        snprintf(name, sizeof(name), "%s", name_str + 1);
        name_str = strchr(name, ')');
        if (name_str != NULL)
            *name_str = '\0';
        else
            name[NAME_LEN - 1] = '\0';
    } else {
        memset(filepath, 0, FILE_LEN);
        strncat(filepath, "/proc/", FILE_LEN - 1);
        strncat(filepath, pid, FILE_LEN - 1);
        strncat(filepath, "/cmdline", FILE_LEN - 1);
        memset(name, 0, sizeof(name));
        fd = open(filepath, O_RDONLY);
        if (fd < 0) {
            printf("open file(%s) error: %m\n", filepath);
        } else {
            num = read(fd, name, NAME_LEN - 1);
            if (num <= 0) {
                printf("read file(%s) error: %m\n", filepath);
            }
            close(fd);
        }
    }
    printf("Parent's name is %s\n", name);
    fflush(stdout);  
    fd = open(REBOOT_DEV, O_RDWR);
    if (fd < 0) {
        printf("fail to open %s: %m\n", REBOOT_DEV);
        return -1;
    }

    /* flush filesystem metadata and dirty pages before triggering reset */
    sync();

    ret = ioctl(fd, REBOOT_IOC_RESET);
    if (ret < 0) {
        printf("ioctl REBOOT_IOC_RESET failed: %m\n");
        close(fd);
        return -1;
    }

    printf("reset command sent successfully\n");
    close(fd);
    return 0;
}
