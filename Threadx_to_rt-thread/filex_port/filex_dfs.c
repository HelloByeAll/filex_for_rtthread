/*
 * @Author: Hello
 * @Date: 2021-07-05 14:33:43
 * @LastEditors: Hello
 * @LastEditTime: 2021-07-13 18:18:15
 * @FilePath: \Threadx_to_rt-thread\filex_port\filex_dfs.c
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "dfs_file.h"
#include "fx_api.h"
#include "rtdbg.h"
// #ifndef FX_DFS_FILEX_MAX_MEDIA_NUM
// #define FX_DFS_FILEX_MAX_MEDIA_NUM 5
// #endif // !FX_DFS_FILEX_MAX_MEDIA_NUM

#ifndef RT_DFS_FILEX_MAX_SECTOR_SIZE
#define RT_DFS_FILEX_MAX_SECTOR_SIZE 4096
#endif // ! RT_DFS_FILEX_MAX_SECTOR_SIZE

#define RT_DFS_SD_SECTOR_OFFSET 0x2000
ALIGN(32)
static char media_memory[RT_DFS_FILEX_MAX_SECTOR_SIZE];
static FX_MEDIA *disk[5] = {0};

static FX_MEDIA *get_disk(rt_device_t dev)
{
    for (int i = 0; i < 5; i++)
    {
        if (disk[i] == NULL)
            continue;
        struct dfs_filesystem *fs = (struct dfs_filesystem *)disk[i]->rt_dfs;
        if (dev == fs->dev_id)
            return disk[i];
    }

    return NULL;
}

static void unmount_disk(FX_MEDIA *fs)
{
    for (int i = 0; i < 5; i++)
    {
        if (disk[i] == fs)
            disk[i] = 0;
    }
}

static int mount_disk(FX_MEDIA *fs)
{
    for (int i = 0; i < 5; i++)
    {
        if (disk[i] == 0)
        {
            disk[i] = fs;
            return i;
        }
    }

    return -1;
}

static void fx_offset_clear(FX_MEDIA *media)
{
#ifndef FX_NO_LOCAL_PATH
    FX_LOCAL_PATH_SETUP
#endif

#ifndef FX_NO_LOCAL_PATH
    if (_tx_thread_current_ptr->tx_thread_filex_ptr)
    {

        /* Yes, there is a local path.  Set the current entry to zero.  */
        ((FX_PATH *)_tx_thread_current_ptr->tx_thread_filex_ptr)->fx_path_current_entry = 0;
    }
    else
    {

        /* Use global default directory.  Set the current entry to 0 in
                        order to pickup the first entry.  */
        media->fx_media_default_path.fx_path_current_entry = 0;
    }
#else

    /* Set the current entry to 0 in order to pickup the first entry.  */
    media_ptr->fx_media_default_path.fx_path_current_entry = 0;
#endif
}

static void fx_dfs_sd_driver(FX_MEDIA *media_ptr)
{
    int i;
    struct dfs_filesystem *fs = media_ptr->rt_dfs;
    rt_device_t dev = fs->dev_id;

    /* Process the driver request specified in the media control block.  */
    // LOG_D("[fx_dfs_sd_driver] %d", media_ptr->fx_media_driver_request);
    switch (media_ptr->fx_media_driver_request)
    {
    case FX_DRIVER_INIT:
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_UNINIT:
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_READ:
    {
        rt_uint32_t readnum;
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        //TX_DISABLE /* disable interrupts */
        if ((readnum = dev->read(dev, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors - RT_DFS_SD_SECTOR_OFFSET,
                                 media_ptr->fx_media_driver_buffer, media_ptr->fx_media_driver_sectors)) > 0)
        {
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            // LOG_HEX("sd0", 16, media_ptr->fx_media_driver_buffer, readnum * 512);
        }
        //TX_RESTORE /* restore interrupts */

        break;
    }

    case FX_DRIVER_WRITE:
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        //TX_DISABLE /* disable interrupts */
        if (dev->write(dev, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors - RT_DFS_SD_SECTOR_OFFSET,
                       media_ptr->fx_media_driver_buffer, media_ptr->fx_media_driver_sectors) > 0)
        {
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            // LOG_HEX("sd0", 16, media_ptr->fx_media_driver_buffer, readnum * 512);
        }
        break;
    }

    case FX_DRIVER_FLUSH:
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_ABORT:
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_BOOT_READ:
    {
        rt_uint32_t partition_start;
        rt_uint32_t partition_size;
        rt_int32_t status;

        /* the boot sector is the sector 0 */
        dev->read(dev, -RT_DFS_SD_SECTOR_OFFSET, media_ptr->fx_media_driver_buffer, 1);
        // LOG_HEX("sd", 16, media_ptr->fx_media_driver_buffer, 512);

        partition_start = 0;

        status = _fx_partition_offset_calculate(media_ptr->fx_media_driver_buffer, 0,
                                                &partition_start, &partition_size);

        /* Check partition read error.  */
        if (status)
        {
            /* Unsuccessful driver request.  */
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            LOG_E("Check partition read error.");
            return;
        }

        if (partition_start)
        {
            LOG_I("partition_start  %d", partition_start);

            status = dev->read(dev, partition_start - RT_DFS_SD_SECTOR_OFFSET, media_ptr->fx_media_driver_buffer, 1);

            // LOG_HEX("sd1", 16, media_ptr->fx_media_driver_buffer, 512);

            if (status == 0)
            {
                /* Unsuccessful driver request.  */
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                LOG_E("Check partition read error.");
                return;
            }
        }

        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_BOOT_WRITE:
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;

        if (dev->write(dev, -RT_DFS_SD_SECTOR_OFFSET, media_ptr->fx_media_driver_buffer, 1) > 0)
        {
            media_ptr->fx_media_driver_status = FX_SUCCESS;
        }

        break;
    }

    default:
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        break;
    }
    }
}

static int dfs_filex_mount(struct dfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    LOG_I("dfs_filex_mount");
    UINT status;
    UINT bytes_per_sector;
    FX_MEDIA *fx_media_disk = RT_NULL;
    static int i = 0;

    if (i == 0)
    {
        fx_system_initialize();
        i++;
    }

    if (fs->data != RT_NULL)
        return -EEXIST;

    char *name = rt_calloc(1, rt_strlen(fs->dev_id->parent.name) + 10);
    rt_sprintf(name, "%s_filex", fs->dev_id->parent.name);
    fx_media_disk = get_disk(fs->dev_id);

    if (fx_media_disk == RT_NULL)
        fx_media_disk = rt_calloc(1, sizeof(FX_MEDIA));

    if (fx_media_disk == RT_NULL)
    {
        status = RT_EFULL;
        goto filex_dfs_err;
    }

    fs->data = fx_media_disk;
    fx_media_disk->rt_dfs = fs;

    if ((status = fx_media_open(fx_media_disk, name, fx_dfs_sd_driver, FX_NULL, media_memory, RT_DFS_FILEX_MAX_SECTOR_SIZE)) != FX_SUCCESS)
    {
        fs->data = 0;
        goto filex_dfs_err;
    }

    if (mount_disk(fx_media_disk) < 0)
    {
        fs->data = 0;
        status = RT_EFULL;
        goto filex_dfs_err;
    }

    LOG_W("fx_media_open  status = %X", status);
    return FX_SUCCESS;

filex_dfs_err:
    if (fx_media_disk != RT_NULL)
        rt_free(fx_media_disk);

    rt_free(name);
    LOG_E("filex_dfs_err %X", status);
    return -status;
}

static int dfs_filex_unmount(struct dfs_filesystem *fs)
{
    LOG_I("dfs_filex_unmount");
    UINT status = RT_EOK;
    FX_MEDIA *fx_media_disk = fs->data;

    if (fx_media_disk)
    {
        unmount_disk(fx_media_disk);
        status = fx_media_close(fx_media_disk);
        fx_media_disk = RT_NULL;
    }
    else
        LOG_W("filex not mounted yet!");

    return -status;
}

static int dfs_filex_mkfs(rt_device_t devid)
{
    UINT status;
    struct rt_device_blk_geometry geometry;
    LOG_I("dfs_filex_mkfs");

    rt_device_open(devid, RT_DEVICE_OFLAG_RDWR);

    rt_device_control(devid, RT_DEVICE_CTRL_BLK_GETGEOME, &geometry);

    FX_MEDIA *media = get_disk(devid);

    if (media == RT_NULL)
    {
        LOG_E("media == RT_NULL");
        media = rt_malloc_align(sizeof(FX_MEDIA), 32);
        mount_disk(media);
        struct dfs_filesystem *fs = rt_malloc_align(sizeof(struct dfs_filesystem), 32);
        fs->dev_id = devid;
        media->rt_dfs = fs;
    }
    else
        fx_media_close(media);

    status = fx_media_format(media,
                             fx_dfs_sd_driver,
                             FX_NULL,
                             media_memory,
                             RT_DFS_FILEX_MAX_SECTOR_SIZE,
                             "fx_volume",
                             1,
                             32,
                             0,
                             10000,
                             512, 1, 1, 1);

    if (status != FX_SUCCESS)
    {
        LOG_E("dfs_filex_mkfs failed %X", status);
    }

    return -status;
}

static int dfs_filex_statfs(struct dfs_filesystem *fs, struct statfs *buf)
{
    LOG_I("dfs_filex_statfs");
    ULONG64 available_bytes;
    RT_ASSERT(fs != RT_NULL);
    RT_ASSERT(buf != RT_NULL);

    FX_MEDIA *media = (FX_MEDIA *)fs->data;
    if (media == RT_NULL)
        return -EEXIST;

    fx_media_extended_space_available(media, &available_bytes);

    buf->f_bfree = available_bytes / 512;
    buf->f_blocks = media->fx_media_total_sectors - media->fx_media_data_sector_start;
    buf->f_bsize = media->fx_media_bytes_per_sector;

    return RT_EOK;
}

static int dfs_filex_unlink(struct dfs_filesystem *fs, const char *pathname)
{
    UINT status;
    LOG_I("dfs_filex_unlink");
    FX_MEDIA *media = (FX_MEDIA *)fs->data;

    status = fx_directory_name_test(media, pathname);
    LOG_I("fx_directory_name_test %s  %X", pathname, status);
    if (status == FX_SUCCESS)
    {
        status = fx_directory_delete(media, pathname);

        LOG_W("fx_directory_delete %X", status);
    }

    else if (status == FX_NOT_DIRECTORY)
    {
        status = fx_file_delete(media, pathname);
        LOG_I("fx_file_delete %X", status);
    }

    if (status == FX_SUCCESS)
        fx_media_flush(media);

    return -status;
}

static int dfs_filex_stat(struct dfs_filesystem *fs, const char *filename, struct stat *st)
{
    LOG_I("dfs_filex_stat");
    int status;
    UINT attributes;
    ULONG size;
    UINT year;
    UINT month;
    UINT day;
    UINT hour;
    UINT minute;
    UINT second;
    FX_MEDIA *media = (FX_MEDIA *)fs->data;
    RT_ASSERT(media != RT_NULL);

    if ((status = fx_directory_information_get(media,
                                               filename,
                                               &attributes,
                                               &size,
                                               &year, &month, &day,
                                               &hour, &minute, &second)) != FX_SUCCESS)
    {
        return -status;
    }

    st->st_dev = 0;
    st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;

    if (attributes & FX_DIRECTORY)
    {
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }
    if (attributes & FX_READ_ONLY)
        st->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

    st->st_size = size;

    LOG_I("time %d %d %d %d %d %d", year, month, day, hour, minute, second);
    /* get st_mtime. */
    {
        struct tm tm_file;
        memset(&tm_file, 0, sizeof(tm_file));

        tm_file.tm_year = year - 1900; /* Years since 1900 */
        tm_file.tm_mon = month;        /* Months *since* january: 0-11 */
        tm_file.tm_mday = day;         /* Day of the month: 1-31 */
        tm_file.tm_hour = hour;        /* Hours since midnight: 0-23 */
        tm_file.tm_min = minute;       /* Minutes: 0-59 */
        tm_file.tm_sec = second;       /* Seconds: 0-59 */

        st->st_mtime = mktime(&tm_file);
    } /* get st_mtime. */

    return RT_EOK;
}

static int dfs_filex_rename(struct dfs_filesystem *fs, const char *oldpath, const char *newpath)
{
    LOG_I("dfs_filex_rename");

    UINT status;
    FX_MEDIA *media = (FX_MEDIA *)fs->data;

    status = fx_directory_name_test(media, oldpath);
    LOG_I("fx_directory_name_test oldpath:%s  newpath:%s  %X", oldpath, newpath, status);
    if (status == FX_SUCCESS)
    {
        status = fx_directory_rename(media, oldpath, newpath);

        LOG_W("fx_directory_rename %X", status);
    }

    else if (status == FX_NOT_DIRECTORY)
    {
        status = fx_file_rename(media, oldpath, newpath);
        LOG_I("fx_file_rename %X", status);
    }

    if (status == FX_SUCCESS)
        fx_media_flush(media);

    return -status;
}

/* *************************** */
static int dfs_filex_open(struct dfs_fd *fd)
{
    LOG_I("dfs_filex_open");
    FX_MEDIA *media;
    FX_FILE *fx_file;
    UINT status;
    UINT open_type = 0;
    media = (FX_MEDIA *)fd->fs->data;

    if (fd->flags & O_DIRECTORY)
    {
        // status = fx_directory_default_set(media, fd->path);

        if (((status = fx_directory_name_test(media, fd->path)) == FX_SUCCESS) || (fd->path[0] == '/' && fd->path[1] == '\0'))
        {
            LOG_I("%s  is a directory", fd->path);
            return RT_EOK;
        }
        else
        {
            if (status == FX_NOT_FOUND && (fd->flags & O_CREAT))
            {
                status = fx_directory_create(media, fd->path);
                return -status;
            }

            LOG_E("fx_directory_name_test path:%s   fd->flags %X!=%X %X", fd->path, fd->flags, O_CREAT, status);
            return -ENOTDIR;
        }
    }
    else
    {
        fx_file = rt_malloc_align(sizeof(FX_FILE), 32);

        if (fx_file != RT_NULL)
        {
            fd->data = fx_file;
        }

        if (fd->flags & O_RDONLY) /* 0 */
            open_type = FX_OPEN_FOR_READ;
        if (fd->flags & O_WRONLY) /* 1 */
            open_type = FX_OPEN_FOR_WRITE;
        if (fd->flags & O_RDWR) /* 2 */
            open_type = FX_OPEN_FOR_READ | FX_OPEN_FOR_WRITE;

        status = fx_file_open(media, fx_file, fd->path, open_type);

        if (status == FX_NOT_FOUND)
        {
            status = fx_file_create(media, fd->path);

            if (status == FX_SUCCESS)
                status = fx_file_open(media, fx_file, fd->path, open_type);
        }

        if (status != FX_SUCCESS)
        {
            rt_free_align(fx_file);
            fd->data = RT_NULL;
        }
        return -status;
    }
}

static int dfs_filex_close(struct dfs_fd *fd)
{
    LOG_I("dfs_filex_close");
    UINT status;
    FX_FILE *fx_file = fd->data;

    if (fx_file == RT_NULL)
    {
        return -EINVAL;
    }

    fx_media_flush(fd->fs->data);
    status = fx_file_close(fx_file);
    fd->pos = 0;

    if (status != FX_SUCCESS)
    {
        return -status;
    }

    return RT_EOK;
}

static int dfs_filex_ioctl(struct dfs_fd *fd, int cmd, void *args)
{
    LOG_I("dfs_filex_ioctl");
    switch (cmd)
    {
    case RT_FIOFTRUNCATE:
    {
        return 1;
    }
    }
    return -ENOSYS;
}

static int dfs_filex_read(struct dfs_fd *fd, void *buf, size_t count)
{
    LOG_I("dfs_filex_read  %s", fd->path);
    UINT status;
    ULONG read_size;
    FX_MEDIA *media;
    FX_FILE *fx_file = fd->data;

    media = (FX_MEDIA *)fd->fs->data;

    if (fx_file == RT_NULL)
    {
        return -EINVAL;
    }

    if (fd->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }

    status = fx_file_read(fx_file, buf, count, &read_size);
    fd->pos += read_size;

    if (status != FX_SUCCESS)
    {
        if (status == FX_END_OF_FILE)
            return 0;
    }

    LOG_I("read_size %d  %d ", count, read_size);
    return read_size;
}

static int dfs_filex_write(struct dfs_fd *fd, const void *buf, size_t count)
{
    LOG_I("dfs_filex_write");

    UINT status;
    FX_FILE *fx_file = fd->data;

    if (fx_file == RT_NULL)
    {
        return -EINVAL;
    }

    LOG_HEX("wr", 16, buf, count);
    status = fx_file_write(fx_file, buf, count);

    if (status != FX_SUCCESS)
    {
        LOG_E("dfs_filex_write failed %X", status);
        return -status;
    }

    return RT_EOK;
}

static int dfs_filex_flush(struct dfs_fd *fd)
{
    LOG_I("dfs_filex_flush");
    UINT status;
    FX_MEDIA *media = (FX_MEDIA *)fd->fs->data;

    status = fx_media_flush(media);

    return -status;
}

static int dfs_filex_lseek(struct dfs_fd *fd, off_t offset)
{
    LOG_I("dfs_filex_lseek");
    UINT status;
    FX_MEDIA *media = (FX_MEDIA *)fd->fs->data;

    if (fd->type == FT_REGULAR)
    {
        FX_FILE *fx_file = fd->data;
        status = fx_file_seek(fx_file, offset);
        if (status == FX_SUCCESS)
        {
            fd->pos = fx_file->fx_file_current_file_offset;
            return fd->pos;
        }
    }
    else if (fd->type == FT_DIRECTORY)
    {
        offset /= sizeof(struct dirent);

        char entry_name[FX_MAX_LONG_NAME_LEN];
        if ((status = fx_directory_first_entry_find(media, entry_name)) != FX_SUCCESS || offset < 0)
        {
            return -EIO;
        }

        if (offset == 0)
            fx_offset_clear(media);

        for (int i = 1; i < offset; i++)
        {
            if ((status = fx_directory_next_entry_find(media, entry_name)) != FX_SUCCESS)
                return -status;
        };

        return RT_EOK;
    }

    return -status;
}

static int dfs_filex_getdents(struct dfs_fd *fd, struct dirent *dirp, uint32_t count)
{
    LOG_I("dfs_filex_getdents");

    FX_MEDIA *media;
    rt_uint32_t result;
    rt_uint32_t index;
    rt_uint32_t attributes;
    ULONG size;
    UINT year;
    UINT month;
    UINT day;
    UINT hour;
    UINT minute;
    UINT second;
    struct dirent *d;

    media = (FX_MEDIA *)(fd->fs->data);
    RT_ASSERT(media != RT_NULL);

    /* make integer count */
    count = (count / sizeof(struct dirent)) * sizeof(struct dirent);
    if (count == 0)
        return -EINVAL;

    index = 0;
    while (1)
    {
        char entry_name[FX_MAX_LONG_NAME_LEN];

        d = dirp + index;

        result = fx_directory_next_full_entry_find(media,
                                                   entry_name,
                                                   &attributes,
                                                   &size,
                                                   &year, &month, &day,
                                                   &hour, &minute, &second);

        if (result != FX_SUCCESS || entry_name[0] == 0)
            break;

        d->d_type = DT_UNKNOWN;
        if (attributes & FX_DIRECTORY)
            d->d_type = DT_DIR;
        else
            d->d_type = DT_REG;

        d->d_namlen = (rt_uint8_t)rt_strlen(entry_name);
        d->d_reclen = (rt_uint16_t)sizeof(struct dirent);
        rt_strncpy(d->d_name, entry_name, rt_strlen(entry_name) + 1);

        index++;
        if (index * sizeof(struct dirent) >= count)
            break;
    }

    if (index == 0)
    {
        fx_offset_clear(media);

        fd->pos = 0;

        return -result;
    }

    fd->pos += index * sizeof(struct dirent);

    return index * sizeof(struct dirent);
}

static const struct dfs_file_ops dfs_filex_fops =
    {
        dfs_filex_open,
        dfs_filex_close,
        dfs_filex_ioctl,
        dfs_filex_read,
        dfs_filex_write,
        dfs_filex_flush,
        dfs_filex_lseek,
        dfs_filex_getdents,
        RT_NULL, /* poll interface */
};

static const struct dfs_filesystem_ops dfs_filex =
    {
        "filex",
        DFS_FS_FLAG_DEFAULT,
        &dfs_filex_fops,
        dfs_filex_mount,
        dfs_filex_unmount,
        dfs_filex_mkfs,
        dfs_filex_statfs,
        dfs_filex_unlink,
        dfs_filex_stat,
        dfs_filex_rename,
};

int filex_init(void)
{
    /* register fatfs file system */
    dfs_register(&dfs_filex);

    return 0;
}
INIT_COMPONENT_EXPORT(filex_init);