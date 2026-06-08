#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include "elf_loader.h"
#include "ampCtrl.h"
#include "kernelmap.h"
#include <time.h>
#include <sys/eventfd.h>

#define PAGE_SIZE ((size_t)getpagesize())
#define MAX_CPU3_COREDUMP_SIZE  (20*1024*1024) /*  20M bytes */ 
#define MODULE_PATH "/usr/local/app/amp_sgi.ko"
#define DEVICE_PATH "/dev/imx6q_amp"
#define COREDUMP_FILE "/usr/local/app/cpu3_coredump"
#define COREDUMP_MAGIC  0x434f5245u
#define AMP_SGI_SET_COREDUMP_EFD _IOW('z', 0x01, int)

#define ALIGN_DOWN(x,align)     (x & (align-1))
#define ALIGN_UP(x,align)       ((x + align -1) & ~(align - 1))

unsigned char Cpu3CoredumpData[MAX_CPU3_COREDUMP_SIZE] = {0};
static int g_dev_fd = -1;
static int g_coredump_efd = -1;
static pthread_t g_coredump_thread;

typedef struct elf_coredump_head
{
  unsigned int magic;
  unsigned int f_size;
} coredump_head_t;

U32 getCpu3CoreDumpData(void)
{
    UADDR Cpu3DumpDataAddr = 0;
    volatile uint8_t *mm = NULL;
    FILE *fp = NULL;
    int fd = 0;
    char filename[256];
    time_t now;
    struct tm *tm_info;
    char time_str[32];
    S32 rc = 0;
    U32 PAGE_CNT = 0;
    U32 uloffset = 0 ; 
    U32 ulPhyBase = 0; 
    VU32 VirtAddr = 0;
    unsigned int bytes_written;
    coredump_head_t head;
    S32 mem_fd = 0;
    rc = getCpu3SectionAddr(".cpu3coredump",&Cpu3DumpDataAddr);
    if(rc != RET_OK)
    {
        printf("get cpu3 core dump Addr failed \r\n");
        return RET_NOK;
    }
    printf("cpu3 core dump addr:0x%lx\r\n",Cpu3DumpDataAddr);

    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem_fd < 0)
    {
        fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
        return RET_NOK;
    }

    uloffset = Cpu3DumpDataAddr % PAGE_SIZE;
    ulPhyBase  = Cpu3DumpDataAddr - uloffset;

    PAGE_CNT = MAX_CPU3_COREDUMP_SIZE / PAGE_SIZE + 1; 

    mm = mmap(NULL, PAGE_SIZE*PAGE_CNT, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, ulPhyBase);
    if (mm == MAP_FAILED) {
        fprintf(stderr, "mmap64(0x%x@0x%x) failed (%d:%s)\n",
                PAGE_SIZE*PAGE_CNT, (uint32_t)(Cpu3DumpDataAddr), errno,strerror(errno));
        return RET_NOK;
    }
    VirtAddr = (UADDR)mm + uloffset;
    printf("CPU3 Core Dump VirtAddr:0x%x ,uloffset:0x%x,ulPhyBase:0x%x,PAGE_CNT:%d\n", VirtAddr,
            uloffset,ulPhyBase,PAGE_CNT);

    memcpy(&head,(unsigned char*)VirtAddr,sizeof(coredump_head_t));
    close(mem_fd);
    if(head.magic != COREDUMP_MAGIC)
    {
        printf("magic is incorrect 0x%08x \n",head.magic);
        munmap((void *)mm, PAGE_SIZE * PAGE_CNT);
        return -1;
    }
    if((head.f_size == 0) || (head.f_size >  MAX_CPU3_COREDUMP_SIZE))
    {
        printf("file size is invalid %d bytes\n",head.f_size);
        munmap((void *)mm, PAGE_SIZE * PAGE_CNT);
        return -1;
    }
    memcpy(&Cpu3CoredumpData[0],(unsigned char*)VirtAddr+sizeof(coredump_head_t),head.f_size);
    msleep(1);
        // Get current time for timestamp
    now = time(NULL);
    tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm_info);
    
    // Create full filename with timestamp
    snprintf(filename, sizeof(filename), "%s_%s.core", COREDUMP_FILE, time_str);
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open coredump file %s: %s\n", 
                filename, strerror(errno));
        munmap((void *)mm, PAGE_SIZE * PAGE_CNT);
        return -1;
    }
    printf("Starting memory dump to %s (size: %d bytes)\n", filename, head.f_size);
    bytes_written = fwrite((void *)Cpu3CoredumpData,1,head.f_size,fp);
    if (bytes_written != head.f_size) {
        fprintf(stderr, "Failed to write complete memory dump: wrote %zd of %d bytes\n",
                bytes_written, head.f_size);
        munmap((void *)mm, PAGE_SIZE * PAGE_CNT);;
        fclose(fp);
        return -1;
    }
    if (fflush(fp) != 0) {
        fprintf(stderr, "Warning: Failed to flush user-space buffer: %s\n", 
                strerror(errno));
    }
    fd = fileno(fp);
    if (fdatasync(fd) == -1) {
        fprintf(stderr, "Warning: Failed to sync data to disk: %s\n", 
                strerror(errno));
    }
    else {
        printf("Memory dump completed successfully: %s\n", filename);
    }
    munmap((void *)mm, PAGE_SIZE * PAGE_CNT);
    fclose(fp);
    return 0;
}

/*
 * Description:
 *   1. Find and install the /usr/local/app/amp_sgi.ko kernel module
 *   2. Open the /dev/imx6q_amp device file
 *   3. Create an eventfd and pass it to the driver via ioctl
 *   4. Spawn a dedicated thread that uses epoll to wait on the eventfd
 *      and calls getCpu3CoreDumpData() when the kernel signals
 */

static void *coredump_efd_loop(void *arg)
{
    int efd = (int)(intptr_t)arg;
    struct epoll_event ev = { .events = EPOLLIN };
    int epfd;

    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) {
        perror("epoll_create1 failed");
        return NULL;
    }

    ev.data.fd = efd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &ev) < 0) {
        perror("epoll_ctl ADD failed");
        close(epfd);
        return NULL;
    }

    printf("coredump epoll thread started (efd=%d, epfd=%d)\n", efd, epfd);

    for (;;) {
        struct epoll_event events[1];
        int n = epoll_wait(epfd, events, 1, -1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait error");
            break;
        }

        if (events[0].events & EPOLLIN) {
            uint64_t val;
            if (read(efd, &val, sizeof(val)) != sizeof(val))
                continue;
            printf("coredump eventfd triggered (count=%llu)\n",
                   (unsigned long long)val);
            getCpu3CoreDumpData();
        }
    }

    close(epfd);
    return NULL;
}

/**
 * @brief Check if a kernel module is already loaded
 * @param module_name Name of the module (without .ko suffix)
 * @return 0 if not loaded, 1 if loaded, -1 on error
 */
static int is_module_loaded(const char *module_name)
{
    FILE *fp;
    char line[256];
    char mod_name[64];
    
    fp = fopen("/proc/modules", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/modules");
        return -1;
    }
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%63s", mod_name) == 1) {
            if (strcmp(mod_name, module_name) == 0) {
                fclose(fp);
                return 1;
            }
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * @brief Install a kernel module
 * @param module_path Full path to the module file
 * @return 0 on success, non-zero on failure
 */
static int install_module(const char *module_path)
{
    char cmd[256];
    int ret;
    struct stat st;
    
    // Check if the module file exists
    if (stat(module_path, &st) != 0) {
        fprintf(stderr, "Module file %s does not exist: %s\n", 
                module_path, strerror(errno));
        return -1;
    }
    
    // Check if it's a regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s is not a regular file\n", module_path);
        return -1;
    }
    
    // Construct the insmod command
    snprintf(cmd, sizeof(cmd), "insmod %s", module_path);
    
    printf("Installing module: %s\n", cmd);
    ret = system(cmd);
    
    if (ret != 0) {
        fprintf(stderr, "Failed to install module %s, return code=%d\n", 
                module_path, ret);
        return -1;
    }
    
    // Wait a short time for the module to finish loading
    usleep(100000); // 100ms
    
    // Verify that the module was successfully loaded
    if (is_module_loaded("amp_sgi") != 1) {
        fprintf(stderr, "Module amp_sgi was not loaded successfully\n");
        return -1;
    }
    
    printf("Module amp_sgi installed successfully\n");
    return 0;
}

/**
 * @brief Install kernel module and set up eventfd+epoll notification for coredump
 * @return 0 on success, -1 on failure
 */
int install_cpu3_coredump_dev(void)
{
    int ret;

    /* Install the kernel module */
    ret = install_module(MODULE_PATH);
    if (ret != 0) {
        fprintf(stderr, "Failed to install module\n");
        return -1;
    }

    /* Open the device file */
    g_dev_fd = open(DEVICE_PATH, O_RDWR);
    if (g_dev_fd < 0) {
        fprintf(stderr, "Failed to open device %s: %s\n",
                DEVICE_PATH, strerror(errno));
        return -1;
    }
    printf("Successfully opened device %s, file descriptor=%d\n",
           DEVICE_PATH, g_dev_fd);

    /* Create the eventfd with non-blocking mode */
    g_coredump_efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (g_coredump_efd < 0) {
        perror("eventfd creation failed");
        close(g_dev_fd);
        g_dev_fd = -1;
        return -1;
    }
    printf("Created coredump eventfd (fd=%d)\n", g_coredump_efd);

    /* Pass the eventfd to the kernel module via ioctl */
    ret = ioctl(g_dev_fd, AMP_SGI_SET_COREDUMP_EFD, &g_coredump_efd);
    if (ret < 0) {
        fprintf(stderr, "ioctl AMP_SGI_SET_COREDUMP_EFD failed: %s\n",
                strerror(errno));
        close(g_coredump_efd);
        close(g_dev_fd);
        g_coredump_efd = -1;
        g_dev_fd = -1;
        return -1;
    }
    printf("Registered eventfd with kernel module\n");

    /* Create the epoll waiter thread */
    ret = pthread_create(&g_coredump_thread, NULL,
                         coredump_efd_loop,
                         (void *)(intptr_t)g_coredump_efd);
    if (ret != 0) {
        errno = ret;
        perror("pthread_create for coredump efd loop failed");
        int clear_fd = -1;
        ioctl(g_dev_fd, AMP_SGI_SET_COREDUMP_EFD, &clear_fd);
        close(g_coredump_efd);
        close(g_dev_fd);
        g_coredump_efd = -1;
        g_dev_fd = -1;
        return -1;
    }

    ret = pthread_setname_np(g_coredump_thread, "t_coredump");
    if (ret != 0) {
        errno = ret;
        perror("pthread_setname_np failed");
    }

    /* Detach the thread — we never join it */
    pthread_detach(g_coredump_thread);

    printf("Coredump notification thread running\n");
    return 0;
}
