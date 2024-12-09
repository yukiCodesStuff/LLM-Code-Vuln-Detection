	switch (cmd) {
	case FDEJECT:
		if (UDRS->fd_ref != 1)
			/* somebody else has this drive open */
			return -EBUSY;
		if (lock_fdc(drive))
			return -EINTR;

		/* do the actual eject. Fails on
		 * non-Sparc architectures */
		ret = fd_eject(UNIT(drive));

		set_bit(FD_DISK_CHANGED_BIT, &UDRS->flags);
		set_bit(FD_VERIFY_BIT, &UDRS->flags);
		process_fd_request();
		return ret;
	case FDCLRPRM:
		if (lock_fdc(drive))
			return -EINTR;
		current_type[drive] = NULL;
		floppy_sizes[drive] = MAX_DISK_SIZE << 1;
		UDRS->keep_data = 0;
		return invalidate_drive(bdev);
	case FDSETPRM:
	case FDDEFPRM:
		return set_geometry(cmd, &inparam.g, drive, type, bdev);
	case FDGETPRM:
		ret = get_floppy_geometry(drive, type,
					  (struct floppy_struct **)&outparam);
		if (ret)
			return ret;
		break;
	case FDMSGON:
		UDP->flags |= FTD_MSG;
		return 0;
	case FDMSGOFF:
		UDP->flags &= ~FTD_MSG;
		return 0;
	case FDFMTBEG:
		if (lock_fdc(drive))
			return -EINTR;
		if (poll_drive(true, FD_RAW_NEED_DISK) == -EINTR)
			return -EINTR;
		ret = UDRS->flags;
		process_fd_request();
		if (ret & FD_VERIFY)
			return -ENODEV;
		if (!(ret & FD_DISK_WRITABLE))
			return -EROFS;
		return 0;
	case FDFMTTRK:
		if (UDRS->fd_ref != 1)
			return -EBUSY;
		return do_format(drive, &inparam.f);
	case FDFMTEND:
	case FDFLUSH:
		if (lock_fdc(drive))
			return -EINTR;
		return invalidate_drive(bdev);
	case FDSETEMSGTRESH:
		UDP->max_errors.reporting = (unsigned short)(param & 0x0f);
		return 0;
	case FDGETMAXERRS:
		outparam = &UDP->max_errors;
		break;
	case FDSETMAXERRS:
		UDP->max_errors = inparam.max_errors;
		break;
	case FDGETDRVTYP:
		outparam = drive_name(type, drive);
		SUPBOUND(size, strlen((const char *)outparam) + 1);
		break;
	case FDSETDRVPRM:
		*UDP = inparam.dp;
		break;
	case FDGETDRVPRM:
		outparam = UDP;
		break;
	case FDPOLLDRVSTAT:
		if (lock_fdc(drive))
			return -EINTR;
		if (poll_drive(true, FD_RAW_NEED_DISK) == -EINTR)
			return -EINTR;
		process_fd_request();
		/* fall through */
	case FDGETDRVSTAT:
		outparam = UDRS;
		break;
	case FDRESET:
		return user_reset_fdc(drive, (int)param, true);
	case FDGETFDCSTAT:
		outparam = UFDCS;
		break;
	case FDWERRORCLR:
		memset(UDRWE, 0, sizeof(*UDRWE));
		return 0;
	case FDWERRORGET:
		outparam = UDRWE;
		break;
	case FDRAWCMD:
		if (type)
			return -EINVAL;
		if (lock_fdc(drive))
			return -EINTR;
		set_floppy(drive);
		i = raw_cmd_ioctl(cmd, (void __user *)param);
		if (i == -EINTR)
			return -EINTR;
		process_fd_request();
		return i;
	case FDTWADDLE:
		if (lock_fdc(drive))
			return -EINTR;
		twaddle();
		process_fd_request();
		return 0;
	default:
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		return fd_copyout((void __user *)param, outparam, size);

	return 0;
}

static int fd_ioctl(struct block_device *bdev, fmode_t mode,
			     unsigned int cmd, unsigned long param)
{
	int ret;

	mutex_lock(&floppy_mutex);
	ret = fd_locked_ioctl(bdev, mode, cmd, param);
	mutex_unlock(&floppy_mutex);

	return ret;
}

#ifdef CONFIG_COMPAT

struct compat_floppy_drive_params {
	char		cmos;
	compat_ulong_t	max_dtr;
	compat_ulong_t	hlt;
	compat_ulong_t	hut;
	compat_ulong_t	srt;
	compat_ulong_t	spinup;
	compat_ulong_t	spindown;
	unsigned char	spindown_offset;
	unsigned char	select_delay;
	unsigned char	rps;
	unsigned char	tracks;
	compat_ulong_t	timeout;
	unsigned char	interleave_sect;
	struct floppy_max_errors max_errors;
	char		flags;
	char		read_track;
	short		autodetect[8];
	compat_int_t	checkfreq;
	compat_int_t	native_format;
};

struct compat_floppy_drive_struct {
	signed char	flags;
	compat_ulong_t	spinup_date;
	compat_ulong_t	select_date;
	compat_ulong_t	first_read_date;
	short		probed_format;
	short		track;
	short		maxblock;
	short		maxtrack;
	compat_int_t	generation;
	compat_int_t	keep_data;
	compat_int_t	fd_ref;
	compat_int_t	fd_device;
	compat_int_t	last_checked;
	compat_caddr_t dmabuf;
	compat_int_t	bufblocks;
};

struct compat_floppy_fdc_state {
	compat_int_t	spec1;
	compat_int_t	spec2;
	compat_int_t	dtr;
	unsigned char	version;
	unsigned char	dor;
	compat_ulong_t	address;
	unsigned int	rawcmd:2;
	unsigned int	reset:1;
	unsigned int	need_configure:1;
	unsigned int	perp_mode:2;
	unsigned int	has_fifo:1;
	unsigned int	driver_version;
	unsigned char	track[4];
};

struct compat_floppy_write_errors {
	unsigned int	write_errors;
	compat_ulong_t	first_error_sector;
	compat_int_t	first_error_generation;
	compat_ulong_t	last_error_sector;
	compat_int_t	last_error_generation;
	compat_uint_t	badness;
};

#define FDSETPRM32 _IOW(2, 0x42, struct compat_floppy_struct)
#define FDDEFPRM32 _IOW(2, 0x43, struct compat_floppy_struct)
#define FDSETDRVPRM32 _IOW(2, 0x90, struct compat_floppy_drive_params)
#define FDGETDRVPRM32 _IOR(2, 0x11, struct compat_floppy_drive_params)
#define FDGETDRVSTAT32 _IOR(2, 0x12, struct compat_floppy_drive_struct)
#define FDPOLLDRVSTAT32 _IOR(2, 0x13, struct compat_floppy_drive_struct)
#define FDGETFDCSTAT32 _IOR(2, 0x15, struct compat_floppy_fdc_state)
#define FDWERRORGET32  _IOR(2, 0x17, struct compat_floppy_write_errors)

static int compat_set_geometry(struct block_device *bdev, fmode_t mode, unsigned int cmd,
		    struct compat_floppy_struct __user *arg)
{
	struct floppy_struct v;
	int drive, type;
	int err;

	BUILD_BUG_ON(offsetof(struct floppy_struct, name) !=
		     offsetof(struct compat_floppy_struct, name));

	if (!(mode & (FMODE_WRITE | FMODE_WRITE_IOCTL)))
		return -EPERM;

	memset(&v, 0, sizeof(struct floppy_struct));
	if (copy_from_user(&v, arg, offsetof(struct floppy_struct, name)))
		return -EFAULT;

	mutex_lock(&floppy_mutex);
	drive = (long)bdev->bd_disk->private_data;
	type = ITYPE(UDRS->fd_device);
	err = set_geometry(cmd == FDSETPRM32 ? FDSETPRM : FDDEFPRM,
			&v, drive, type, bdev);
	mutex_unlock(&floppy_mutex);
	return err;
}

static int compat_get_prm(int drive,
			  struct compat_floppy_struct __user *arg)
{
	struct compat_floppy_struct v;
	struct floppy_struct *p;
	int err;

	memset(&v, 0, sizeof(v));
	mutex_lock(&floppy_mutex);
	err = get_floppy_geometry(drive, ITYPE(UDRS->fd_device), &p);
	if (err) {
		mutex_unlock(&floppy_mutex);
		return err;
	}
	memcpy(&v, p, offsetof(struct floppy_struct, name));
	mutex_unlock(&floppy_mutex);
	if (copy_to_user(arg, &v, sizeof(struct compat_floppy_struct)))
		return -EFAULT;
	return 0;
}

static int compat_setdrvprm(int drive,
			    struct compat_floppy_drive_params __user *arg)
{
	struct compat_floppy_drive_params v;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (copy_from_user(&v, arg, sizeof(struct compat_floppy_drive_params)))
		return -EFAULT;
	mutex_lock(&floppy_mutex);
	UDP->cmos = v.cmos;
	UDP->max_dtr = v.max_dtr;
	UDP->hlt = v.hlt;
	UDP->hut = v.hut;
	UDP->srt = v.srt;
	UDP->spinup = v.spinup;
	UDP->spindown = v.spindown;
	UDP->spindown_offset = v.spindown_offset;
	UDP->select_delay = v.select_delay;
	UDP->rps = v.rps;
	UDP->tracks = v.tracks;
	UDP->timeout = v.timeout;
	UDP->interleave_sect = v.interleave_sect;
	UDP->max_errors = v.max_errors;
	UDP->flags = v.flags;
	UDP->read_track = v.read_track;
	memcpy(UDP->autodetect, v.autodetect, sizeof(v.autodetect));
	UDP->checkfreq = v.checkfreq;
	UDP->native_format = v.native_format;
	mutex_unlock(&floppy_mutex);
	return 0;
}

static int compat_getdrvprm(int drive,
			    struct compat_floppy_drive_params __user *arg)
{
	struct compat_floppy_drive_params v;

	memset(&v, 0, sizeof(struct compat_floppy_drive_params));
	mutex_lock(&floppy_mutex);
	v.cmos = UDP->cmos;
	v.max_dtr = UDP->max_dtr;
	v.hlt = UDP->hlt;
	v.hut = UDP->hut;
	v.srt = UDP->srt;
	v.spinup = UDP->spinup;
	v.spindown = UDP->spindown;
	v.spindown_offset = UDP->spindown_offset;
	v.select_delay = UDP->select_delay;
	v.rps = UDP->rps;
	v.tracks = UDP->tracks;
	v.timeout = UDP->timeout;
	v.interleave_sect = UDP->interleave_sect;
	v.max_errors = UDP->max_errors;
	v.flags = UDP->flags;
	v.read_track = UDP->read_track;
	memcpy(v.autodetect, UDP->autodetect, sizeof(v.autodetect));
	v.checkfreq = UDP->checkfreq;
	v.native_format = UDP->native_format;
	mutex_unlock(&floppy_mutex);

	if (copy_from_user(arg, &v, sizeof(struct compat_floppy_drive_params)))
		return -EFAULT;
	return 0;
}

static int compat_getdrvstat(int drive, bool poll,
			    struct compat_floppy_drive_struct __user *arg)
{
	struct compat_floppy_drive_struct v;

	memset(&v, 0, sizeof(struct compat_floppy_drive_struct));
	mutex_lock(&floppy_mutex);

	if (poll) {
		if (lock_fdc(drive))
			goto Eintr;
		if (poll_drive(true, FD_RAW_NEED_DISK) == -EINTR)
			goto Eintr;
		process_fd_request();
	}
	v.spinup_date = UDRS->spinup_date;
	v.select_date = UDRS->select_date;
	v.first_read_date = UDRS->first_read_date;
	v.probed_format = UDRS->probed_format;
	v.track = UDRS->track;
	v.maxblock = UDRS->maxblock;
	v.maxtrack = UDRS->maxtrack;
	v.generation = UDRS->generation;
	v.keep_data = UDRS->keep_data;
	v.fd_ref = UDRS->fd_ref;
	v.fd_device = UDRS->fd_device;
	v.last_checked = UDRS->last_checked;
	v.dmabuf = (uintptr_t)UDRS->dmabuf;
	v.bufblocks = UDRS->bufblocks;
	mutex_unlock(&floppy_mutex);

	if (copy_from_user(arg, &v, sizeof(struct compat_floppy_drive_struct)))
		return -EFAULT;
	return 0;
Eintr:
	mutex_unlock(&floppy_mutex);
	return -EINTR;
}

static int compat_getfdcstat(int drive,
			    struct compat_floppy_fdc_state __user *arg)
{
	struct compat_floppy_fdc_state v32;
	struct floppy_fdc_state v;

	mutex_lock(&floppy_mutex);
	v = *UFDCS;
	mutex_unlock(&floppy_mutex);

	memset(&v32, 0, sizeof(struct compat_floppy_fdc_state));
	v32.spec1 = v.spec1;
	v32.spec2 = v.spec2;
	v32.dtr = v.dtr;
	v32.version = v.version;
	v32.dor = v.dor;
	v32.address = v.address;
	v32.rawcmd = v.rawcmd;
	v32.reset = v.reset;
	v32.need_configure = v.need_configure;
	v32.perp_mode = v.perp_mode;
	v32.has_fifo = v.has_fifo;
	v32.driver_version = v.driver_version;
	memcpy(v32.track, v.track, 4);
	if (copy_to_user(arg, &v32, sizeof(struct compat_floppy_fdc_state)))
		return -EFAULT;
	return 0;
}

static int compat_werrorget(int drive,
			    struct compat_floppy_write_errors __user *arg)
{
	struct compat_floppy_write_errors v32;
	struct floppy_write_errors v;

	memset(&v32, 0, sizeof(struct compat_floppy_write_errors));
	mutex_lock(&floppy_mutex);
	v = *UDRWE;
	mutex_unlock(&floppy_mutex);
	v32.write_errors = v.write_errors;
	v32.first_error_sector = v.first_error_sector;
	v32.first_error_generation = v.first_error_generation;
	v32.last_error_sector = v.last_error_sector;
	v32.last_error_generation = v.last_error_generation;
	v32.badness = v.badness;
	if (copy_to_user(arg, &v32, sizeof(struct compat_floppy_write_errors)))
		return -EFAULT;
	return 0;
}

static int fd_compat_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd,
		    unsigned long param)
{
	int drive = (long)bdev->bd_disk->private_data;
	switch (cmd) {
	case FDMSGON:
	case FDMSGOFF:
	case FDSETEMSGTRESH:
	case FDFLUSH:
	case FDWERRORCLR:
	case FDEJECT:
	case FDCLRPRM:
	case FDFMTBEG:
	case FDRESET:
	case FDTWADDLE:
		return fd_ioctl(bdev, mode, cmd, param);
	case FDSETMAXERRS:
	case FDGETMAXERRS:
	case FDGETDRVTYP:
	case FDFMTEND:
	case FDFMTTRK:
	case FDRAWCMD:
		return fd_ioctl(bdev, mode, cmd,
				(unsigned long)compat_ptr(param));
	case FDSETPRM32:
	case FDDEFPRM32:
		return compat_set_geometry(bdev, mode, cmd, compat_ptr(param));
	case FDGETPRM32:
		return compat_get_prm(drive, compat_ptr(param));
	case FDSETDRVPRM32:
		return compat_setdrvprm(drive, compat_ptr(param));
	case FDGETDRVPRM32:
		return compat_getdrvprm(drive, compat_ptr(param));
	case FDPOLLDRVSTAT32:
		return compat_getdrvstat(drive, true, compat_ptr(param));
	case FDGETDRVSTAT32:
		return compat_getdrvstat(drive, false, compat_ptr(param));
	case FDGETFDCSTAT32:
		return compat_getfdcstat(drive, compat_ptr(param));
	case FDWERRORGET32:
		return compat_werrorget(drive, compat_ptr(param));
	}
	return -EINVAL;
}
#endif

static void __init config_types(void)
{
	bool has_drive = false;
	int drive;

	/* read drive info out of physical CMOS */
	drive = 0;
	if (!UDP->cmos)
		UDP->cmos = FLOPPY0_TYPE;
	drive = 1;
	if (!UDP->cmos && FLOPPY1_TYPE)
		UDP->cmos = FLOPPY1_TYPE;

	/* FIXME: additional physical CMOS drive detection should go here */

	for (drive = 0; drive < N_DRIVE; drive++) {
		unsigned int type = UDP->cmos;
		struct floppy_drive_params *params;
		const char *name = NULL;
		char temparea[32];

		if (type < ARRAY_SIZE(default_drive_params)) {
			params = &default_drive_params[type].params;
			if (type) {
				name = default_drive_params[type].name;
				allowed_drive_mask |= 1 << drive;
			} else
				allowed_drive_mask &= ~(1 << drive);
		} else {
			params = &default_drive_params[0].params;
			snprintf(temparea, sizeof(temparea),
				 "unknown type %d (usb?)", type);
			name = temparea;
		}
		if (name) {
			const char *prepend;
			if (!has_drive) {
				prepend = "";
				has_drive = true;
				pr_info("Floppy drive(s):");
			} else {
				prepend = ",";
			}

			pr_cont("%s fd%d is %s", prepend, drive, name);
		}
		*UDP = *params;
	}

	if (has_drive)
		pr_cont("\n");
}

static void floppy_release(struct gendisk *disk, fmode_t mode)
{
	int drive = (long)disk->private_data;

	mutex_lock(&floppy_mutex);
	mutex_lock(&open_lock);
	if (!UDRS->fd_ref--) {
		DPRINT("floppy_release with fd_ref == 0");
		UDRS->fd_ref = 0;
	}
	if (!UDRS->fd_ref)
		opened_bdev[drive] = NULL;
	mutex_unlock(&open_lock);
	mutex_unlock(&floppy_mutex);
}

/*
 * floppy_open check for aliasing (/dev/fd0 can be the same as
 * /dev/PS0 etc), and disallows simultaneous access to the same
 * drive with different device numbers.
 */
static int floppy_open(struct block_device *bdev, fmode_t mode)
{
	int drive = (long)bdev->bd_disk->private_data;
	int old_dev, new_dev;
	int try;
	int res = -EBUSY;
	char *tmp;

	mutex_lock(&floppy_mutex);
	mutex_lock(&open_lock);
	old_dev = UDRS->fd_device;
	if (opened_bdev[drive] && opened_bdev[drive] != bdev)
		goto out2;

	if (!UDRS->fd_ref && (UDP->flags & FD_BROKEN_DCL)) {
		set_bit(FD_DISK_CHANGED_BIT, &UDRS->flags);
		set_bit(FD_VERIFY_BIT, &UDRS->flags);
	}

	UDRS->fd_ref++;

	opened_bdev[drive] = bdev;

	res = -ENXIO;

	if (!floppy_track_buffer) {
		/* if opening an ED drive, reserve a big buffer,
		 * else reserve a small one */
		if ((UDP->cmos == 6) || (UDP->cmos == 5))
			try = 64;	/* Only 48 actually useful */
		else
			try = 32;	/* Only 24 actually useful */

		tmp = (char *)fd_dma_mem_alloc(1024 * try);
		if (!tmp && !floppy_track_buffer) {
			try >>= 1;	/* buffer only one side */
			INFBOUND(try, 16);
			tmp = (char *)fd_dma_mem_alloc(1024 * try);
		}
		if (!tmp && !floppy_track_buffer)
			fallback_on_nodma_alloc(&tmp, 2048 * try);
		if (!tmp && !floppy_track_buffer) {
			DPRINT("Unable to allocate DMA memory\n");
			goto out;
		}
		if (floppy_track_buffer) {
			if (tmp)
				fd_dma_mem_free((unsigned long)tmp, try * 1024);
		} else {
			buffer_min = buffer_max = -1;
			floppy_track_buffer = tmp;
			max_buffer_sectors = try;
		}
	}

	new_dev = MINOR(bdev->bd_dev);
	UDRS->fd_device = new_dev;
	set_capacity(disks[drive], floppy_sizes[new_dev]);
	if (old_dev != -1 && old_dev != new_dev) {
		if (buffer_drive == drive)
			buffer_track = -1;
	}

	if (UFDCS->rawcmd == 1)
		UFDCS->rawcmd = 2;

	if (!(mode & FMODE_NDELAY)) {
		if (mode & (FMODE_READ|FMODE_WRITE)) {
			UDRS->last_checked = 0;
			clear_bit(FD_OPEN_SHOULD_FAIL_BIT, &UDRS->flags);
			check_disk_change(bdev);
			if (test_bit(FD_DISK_CHANGED_BIT, &UDRS->flags))
				goto out;
			if (test_bit(FD_OPEN_SHOULD_FAIL_BIT, &UDRS->flags))
				goto out;
		}
		res = -EROFS;
		if ((mode & FMODE_WRITE) &&
		    !test_bit(FD_DISK_WRITABLE_BIT, &UDRS->flags))
			goto out;
	}
	mutex_unlock(&open_lock);
	mutex_unlock(&floppy_mutex);
	return 0;
out:
	UDRS->fd_ref--;

	if (!UDRS->fd_ref)
		opened_bdev[drive] = NULL;
out2:
	mutex_unlock(&open_lock);
	mutex_unlock(&floppy_mutex);
	return res;
}

/*
 * Check if the disk has been changed or if a change has been faked.
 */
static unsigned int floppy_check_events(struct gendisk *disk,
					unsigned int clearing)
{
	int drive = (long)disk->private_data;

	if (test_bit(FD_DISK_CHANGED_BIT, &UDRS->flags) ||
	    test_bit(FD_VERIFY_BIT, &UDRS->flags))
		return DISK_EVENT_MEDIA_CHANGE;

	if (time_after(jiffies, UDRS->last_checked + UDP->checkfreq)) {
		if (lock_fdc(drive))
			return -EINTR;
		poll_drive(false, 0);
		process_fd_request();
	}

	if (test_bit(FD_DISK_CHANGED_BIT, &UDRS->flags) ||
	    test_bit(FD_VERIFY_BIT, &UDRS->flags) ||
	    test_bit(drive, &fake_change) ||
	    drive_no_geom(drive))
		return DISK_EVENT_MEDIA_CHANGE;
	return 0;
}

/*
 * This implements "read block 0" for floppy_revalidate().
 * Needed for format autodetection, checking whether there is
 * a disk in the drive, and whether that disk is writable.
 */

struct rb0_cbdata {
	int drive;
	struct completion complete;
};

static void floppy_rb0_cb(struct bio *bio)
{
	struct rb0_cbdata *cbdata = (struct rb0_cbdata *)bio->bi_private;
	int drive = cbdata->drive;

	if (bio->bi_status) {
		pr_info("floppy: error %d while reading block 0\n",
			bio->bi_status);
		set_bit(FD_OPEN_SHOULD_FAIL_BIT, &UDRS->flags);
	}
	complete(&cbdata->complete);
}

static int __floppy_read_block_0(struct block_device *bdev, int drive)
{
	struct bio bio;
	struct bio_vec bio_vec;
	struct page *page;
	struct rb0_cbdata cbdata;
	size_t size;

	page = alloc_page(GFP_NOIO);
	if (!page) {
		process_fd_request();
		return -ENOMEM;
	}

	size = bdev->bd_block_size;
	if (!size)
		size = 1024;

	cbdata.drive = drive;

	bio_init(&bio, &bio_vec, 1);
	bio_set_dev(&bio, bdev);
	bio_add_page(&bio, page, size, 0);

	bio.bi_iter.bi_sector = 0;
	bio.bi_flags |= (1 << BIO_QUIET);
	bio.bi_private = &cbdata;
	bio.bi_end_io = floppy_rb0_cb;
	bio_set_op_attrs(&bio, REQ_OP_READ, 0);

	submit_bio(&bio);
	process_fd_request();

	init_completion(&cbdata.complete);
	wait_for_completion(&cbdata.complete);

	__free_page(page);

	return 0;
}

/* revalidate the floppy disk, i.e. trigger format autodetection by reading
 * the bootblock (block 0). "Autodetection" is also needed to check whether
 * there is a disk in the drive at all... Thus we also do it for fixed
 * geometry formats */
static int floppy_revalidate(struct gendisk *disk)
{
	int drive = (long)disk->private_data;
	int cf;
	int res = 0;

	if (test_bit(FD_DISK_CHANGED_BIT, &UDRS->flags) ||
	    test_bit(FD_VERIFY_BIT, &UDRS->flags) ||
	    test_bit(drive, &fake_change) ||
	    drive_no_geom(drive)) {
		if (WARN(atomic_read(&usage_count) == 0,
			 "VFS: revalidate called on non-open device.\n"))
			return -EFAULT;

		res = lock_fdc(drive);
		if (res)
			return res;
		cf = (test_bit(FD_DISK_CHANGED_BIT, &UDRS->flags) ||
		      test_bit(FD_VERIFY_BIT, &UDRS->flags));
		if (!(cf || test_bit(drive, &fake_change) || drive_no_geom(drive))) {
			process_fd_request();	/*already done by another thread */
			return 0;
		}
		UDRS->maxblock = 0;
		UDRS->maxtrack = 0;
		if (buffer_drive == drive)
			buffer_track = -1;
		clear_bit(drive, &fake_change);
		clear_bit(FD_DISK_CHANGED_BIT, &UDRS->flags);
		if (cf)
			UDRS->generation++;
		if (drive_no_geom(drive)) {
			/* auto-sensing */
			res = __floppy_read_block_0(opened_bdev[drive], drive);
		} else {
			if (cf)
				poll_drive(false, FD_RAW_NEED_DISK);
			process_fd_request();
		}
	}
	set_capacity(disk, floppy_sizes[UDRS->fd_device]);
	return res;
}

static const struct block_device_operations floppy_fops = {
	.owner			= THIS_MODULE,
	.open			= floppy_open,
	.release		= floppy_release,
	.ioctl			= fd_ioctl,
	.getgeo			= fd_getgeo,
	.check_events		= floppy_check_events,
	.revalidate_disk	= floppy_revalidate,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= fd_compat_ioctl,
#endif
};

/*
 * Floppy Driver initialization
 * =============================
 */

/* Determine the floppy disk controller type */
/* This routine was written by David C. Niemi */
static char __init get_fdc_version(void)
{
	int r;

	output_byte(FD_DUMPREGS);	/* 82072 and better know DUMPREGS */
	if (FDCS->reset)
		return FDC_NONE;
	r = result();
	if (r <= 0x00)
		return FDC_NONE;	/* No FDC present ??? */
	if ((r == 1) && (reply_buffer[0] == 0x80)) {
		pr_info("FDC %d is an 8272A\n", fdc);
		return FDC_8272A;	/* 8272a/765 don't know DUMPREGS */
	}
	if (r != 10) {
		pr_info("FDC %d init: DUMPREGS: unexpected return of %d bytes.\n",
			fdc, r);
		return FDC_UNKNOWN;
	}

	if (!fdc_configure()) {
		pr_info("FDC %d is an 82072\n", fdc);
		return FDC_82072;	/* 82072 doesn't know CONFIGURE */
	}

	output_byte(FD_PERPENDICULAR);
	if (need_more_output() == MORE_OUTPUT) {
		output_byte(0);
	} else {
		pr_info("FDC %d is an 82072A\n", fdc);
		return FDC_82072A;	/* 82072A as found on Sparcs. */
	}

	output_byte(FD_UNLOCK);
	r = result();
	if ((r == 1) && (reply_buffer[0] == 0x80)) {
		pr_info("FDC %d is a pre-1991 82077\n", fdc);
		return FDC_82077_ORIG;	/* Pre-1991 82077, doesn't know
					 * LOCK/UNLOCK */
	}
	if ((r != 1) || (reply_buffer[0] != 0x00)) {
		pr_info("FDC %d init: UNLOCK: unexpected return of %d bytes.\n",
			fdc, r);
		return FDC_UNKNOWN;
	}
	output_byte(FD_PARTID);
	r = result();
	if (r != 1) {
		pr_info("FDC %d init: PARTID: unexpected return of %d bytes.\n",
			fdc, r);
		return FDC_UNKNOWN;
	}
	if (reply_buffer[0] == 0x80) {
		pr_info("FDC %d is a post-1991 82077\n", fdc);
		return FDC_82077;	/* Revised 82077AA passes all the tests */
	}
	switch (reply_buffer[0] >> 5) {
	case 0x0:
		/* Either a 82078-1 or a 82078SL running at 5Volt */
		pr_info("FDC %d is an 82078.\n", fdc);
		return FDC_82078;
	case 0x1:
		pr_info("FDC %d is a 44pin 82078\n", fdc);
		return FDC_82078;
	case 0x2:
		pr_info("FDC %d is a S82078B\n", fdc);
		return FDC_S82078B;
	case 0x3:
		pr_info("FDC %d is a National Semiconductor PC87306\n", fdc);
		return FDC_87306;
	default:
		pr_info("FDC %d init: 82078 variant with unknown PARTID=%d.\n",
			fdc, reply_buffer[0] >> 5);
		return FDC_82078_UNKN;
	}
}				/* get_fdc_version */

/* lilo configuration */

static void __init floppy_set_flags(int *ints, int param, int param2)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(default_drive_params); i++) {
		if (param)
			default_drive_params[i].params.flags |= param2;
		else
			default_drive_params[i].params.flags &= ~param2;
	}
	DPRINT("%s flag 0x%x\n", param2 ? "Setting" : "Clearing", param);
}

static void __init daring(int *ints, int param, int param2)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(default_drive_params); i++) {
		if (param) {
			default_drive_params[i].params.select_delay = 0;
			default_drive_params[i].params.flags |=
			    FD_SILENT_DCL_CLEAR;
		} else {
			default_drive_params[i].params.select_delay =
			    2 * HZ / 100;
			default_drive_params[i].params.flags &=
			    ~FD_SILENT_DCL_CLEAR;
		}
	}
	DPRINT("Assuming %s floppy hardware\n", param ? "standard" : "broken");
}

static void __init set_cmos(int *ints, int dummy, int dummy2)
{
	int current_drive = 0;

	if (ints[0] != 2) {
		DPRINT("wrong number of parameters for CMOS\n");
		return;
	}
	current_drive = ints[1];
	if (current_drive < 0 || current_drive >= 8) {
		DPRINT("bad drive for set_cmos\n");
		return;
	}
#if N_FDC > 1
	if (current_drive >= 4 && !FDC2)
		FDC2 = 0x370;
#endif
	DP->cmos = ints[2];
	DPRINT("setting CMOS code to %d\n", ints[2]);
}

static struct param_table {
	const char *name;
	void (*fn) (int *ints, int param, int param2);
	int *var;
	int def_param;
	int param2;
} config_params[] __initdata = {
	{"allowed_drive_mask", NULL, &allowed_drive_mask, 0xff, 0}, /* obsolete */
	{"all_drives", NULL, &allowed_drive_mask, 0xff, 0},	/* obsolete */
	{"asus_pci", NULL, &allowed_drive_mask, 0x33, 0},
	{"irq", NULL, &FLOPPY_IRQ, 6, 0},
	{"dma", NULL, &FLOPPY_DMA, 2, 0},
	{"daring", daring, NULL, 1, 0},
#if N_FDC > 1
	{"two_fdc", NULL, &FDC2, 0x370, 0},
	{"one_fdc", NULL, &FDC2, 0, 0},
#endif
	{"thinkpad", floppy_set_flags, NULL, 1, FD_INVERTED_DCL},
	{"broken_dcl", floppy_set_flags, NULL, 1, FD_BROKEN_DCL},
	{"messages", floppy_set_flags, NULL, 1, FTD_MSG},
	{"silent_dcl_clear", floppy_set_flags, NULL, 1, FD_SILENT_DCL_CLEAR},
	{"debug", floppy_set_flags, NULL, 1, FD_DEBUG},
	{"nodma", NULL, &can_use_virtual_dma, 1, 0},
	{"omnibook", NULL, &can_use_virtual_dma, 1, 0},
	{"yesdma", NULL, &can_use_virtual_dma, 0, 0},
	{"fifo_depth", NULL, &fifo_depth, 0xa, 0},
	{"nofifo", NULL, &no_fifo, 0x20, 0},
	{"usefifo", NULL, &no_fifo, 0, 0},
	{"cmos", set_cmos, NULL, 0, 0},
	{"slow", NULL, &slow_floppy, 1, 0},
	{"unexpected_interrupts", NULL, &print_unex, 1, 0},
	{"no_unexpected_interrupts", NULL, &print_unex, 0, 0},
	{"L40SX", NULL, &print_unex, 0, 0}

	EXTRA_FLOPPY_PARAMS
};

static int __init floppy_setup(char *str)
{
	int i;
	int param;
	int ints[11];

	str = get_options(str, ARRAY_SIZE(ints), ints);
	if (str) {
		for (i = 0; i < ARRAY_SIZE(config_params); i++) {
			if (strcmp(str, config_params[i].name) == 0) {
				if (ints[0])
					param = ints[1];
				else
					param = config_params[i].def_param;
				if (config_params[i].fn)
					config_params[i].fn(ints, param,
							    config_params[i].
							    param2);
				if (config_params[i].var) {
					DPRINT("%s=%d\n", str, param);
					*config_params[i].var = param;
				}
				return 1;
			}
		}
	}
	if (str) {
		DPRINT("unknown floppy option [%s]\n", str);

		DPRINT("allowed options are:");
		for (i = 0; i < ARRAY_SIZE(config_params); i++)
			pr_cont(" %s", config_params[i].name);
		pr_cont("\n");
	} else
		DPRINT("botched floppy option\n");
	DPRINT("Read Documentation/blockdev/floppy.txt\n");
	return 0;
}

static int have_no_fdc = -ENODEV;

static ssize_t floppy_cmos_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct platform_device *p = to_platform_device(dev);
	int drive;

	drive = p->id;
	return sprintf(buf, "%X\n", UDP->cmos);
}

static DEVICE_ATTR(cmos, 0444, floppy_cmos_show, NULL);

static struct attribute *floppy_dev_attrs[] = {
	&dev_attr_cmos.attr,
	NULL
};

ATTRIBUTE_GROUPS(floppy_dev);

static void floppy_device_release(struct device *dev)
{
}

static int floppy_resume(struct device *dev)
{
	int fdc;

	for (fdc = 0; fdc < N_FDC; fdc++)
		if (FDCS->address != -1)
			user_reset_fdc(-1, FD_RESET_ALWAYS, false);

	return 0;
}

static const struct dev_pm_ops floppy_pm_ops = {
	.resume = floppy_resume,
	.restore = floppy_resume,
};

static struct platform_driver floppy_driver = {
	.driver = {
		   .name = "floppy",
		   .pm = &floppy_pm_ops,
	},
};

static struct platform_device floppy_device[N_DRIVE];

static bool floppy_available(int drive)
{
	if (!(allowed_drive_mask & (1 << drive)))
		return false;
	if (fdc_state[FDC(drive)].version == FDC_NONE)
		return false;
	return true;
}

static struct kobject *floppy_find(dev_t dev, int *part, void *data)
{
	int drive = (*part & 3) | ((*part & 0x80) >> 5);
	if (drive >= N_DRIVE || !floppy_available(drive))
		return NULL;
	if (((*part >> 2) & 0x1f) >= ARRAY_SIZE(floppy_type))
		return NULL;
	*part = 0;
	return get_disk_and_module(disks[drive]);
}

static int __init do_floppy_init(void)
{
	int i, unit, drive, err;

	set_debugt();
	interruptjiffies = resultjiffies = jiffies;

#if defined(CONFIG_PPC)
	if (check_legacy_ioport(FDC1))
		return -ENODEV;
#endif

	raw_cmd = NULL;

	floppy_wq = alloc_ordered_workqueue("floppy", 0);
	if (!floppy_wq)
		return -ENOMEM;

	for (drive = 0; drive < N_DRIVE; drive++) {
		disks[drive] = alloc_disk(1);
		if (!disks[drive]) {
			err = -ENOMEM;
			goto out_put_disk;
		}

		disks[drive]->queue = blk_init_queue(do_fd_request, &floppy_lock);
		if (!disks[drive]->queue) {
			err = -ENOMEM;
			goto out_put_disk;
		}

		blk_queue_bounce_limit(disks[drive]->queue, BLK_BOUNCE_HIGH);
		blk_queue_max_hw_sectors(disks[drive]->queue, 64);
		disks[drive]->major = FLOPPY_MAJOR;
		disks[drive]->first_minor = TOMINOR(drive);
		disks[drive]->fops = &floppy_fops;
		sprintf(disks[drive]->disk_name, "fd%d", drive);

		timer_setup(&motor_off_timer[drive], motor_off_callback, 0);
	}

	err = register_blkdev(FLOPPY_MAJOR, "fd");
	if (err)
		goto out_put_disk;

	err = platform_driver_register(&floppy_driver);
	if (err)
		goto out_unreg_blkdev;

	blk_register_region(MKDEV(FLOPPY_MAJOR, 0), 256, THIS_MODULE,
			    floppy_find, NULL, NULL);

	for (i = 0; i < 256; i++)
		if (ITYPE(i))
			floppy_sizes[i] = floppy_type[ITYPE(i)].size;
		else
			floppy_sizes[i] = MAX_DISK_SIZE << 1;

	reschedule_timeout(MAXTIMEOUT, "floppy init");
	config_types();

	for (i = 0; i < N_FDC; i++) {
		fdc = i;
		memset(FDCS, 0, sizeof(*FDCS));
		FDCS->dtr = -1;
		FDCS->dor = 0x4;
#if defined(__sparc__) || defined(__mc68000__)
	/*sparcs/sun3x don't have a DOR reset which we can fall back on to */
#ifdef __mc68000__
		if (MACH_IS_SUN3X)
#endif
			FDCS->version = FDC_82072A;
#endif
	}

	use_virtual_dma = can_use_virtual_dma & 1;
	fdc_state[0].address = FDC1;
	if (fdc_state[0].address == -1) {
		cancel_delayed_work(&fd_timeout);
		err = -ENODEV;
		goto out_unreg_region;
	}
#if N_FDC > 1
	fdc_state[1].address = FDC2;
#endif

	fdc = 0;		/* reset fdc in case of unexpected interrupt */
	err = floppy_grab_irq_and_dma();
	if (err) {
		cancel_delayed_work(&fd_timeout);
		err = -EBUSY;
		goto out_unreg_region;
	}

	/* initialise drive state */
	for (drive = 0; drive < N_DRIVE; drive++) {
		memset(UDRS, 0, sizeof(*UDRS));
		memset(UDRWE, 0, sizeof(*UDRWE));
		set_bit(FD_DISK_NEWCHANGE_BIT, &UDRS->flags);
		set_bit(FD_DISK_CHANGED_BIT, &UDRS->flags);
		set_bit(FD_VERIFY_BIT, &UDRS->flags);
		UDRS->fd_device = -1;
		floppy_track_buffer = NULL;
		max_buffer_sectors = 0;
	}
	/*
	 * Small 10 msec delay to let through any interrupt that
	 * initialization might have triggered, to not
	 * confuse detection:
	 */
	msleep(10);

	for (i = 0; i < N_FDC; i++) {
		fdc = i;
		FDCS->driver_version = FD_DRIVER_VERSION;
		for (unit = 0; unit < 4; unit++)
			FDCS->track[unit] = 0;
		if (FDCS->address == -1)
			continue;
		FDCS->rawcmd = 2;
		if (user_reset_fdc(-1, FD_RESET_ALWAYS, false)) {
			/* free ioports reserved by floppy_grab_irq_and_dma() */
			floppy_release_regions(fdc);
			FDCS->address = -1;
			FDCS->version = FDC_NONE;
			continue;
		}
		/* Try to determine the floppy controller type */
		FDCS->version = get_fdc_version();
		if (FDCS->version == FDC_NONE) {
			/* free ioports reserved by floppy_grab_irq_and_dma() */
			floppy_release_regions(fdc);
			FDCS->address = -1;
			continue;
		}
		if (can_use_virtual_dma == 2 && FDCS->version < FDC_82072A)
			can_use_virtual_dma = 0;

		have_no_fdc = 0;
		/* Not all FDCs seem to be able to handle the version command
		 * properly, so force a reset for the standard FDC clones,
		 * to avoid interrupt garbage.
		 */
		user_reset_fdc(-1, FD_RESET_ALWAYS, false);
	}
	fdc = 0;
	cancel_delayed_work(&fd_timeout);
	current_drive = 0;
	initialized = true;
	if (have_no_fdc) {
		DPRINT("no floppy controllers found\n");
		err = have_no_fdc;
		goto out_release_dma;
	}

	for (drive = 0; drive < N_DRIVE; drive++) {
		if (!floppy_available(drive))
			continue;

		floppy_device[drive].name = floppy_device_name;
		floppy_device[drive].id = drive;
		floppy_device[drive].dev.release = floppy_device_release;
		floppy_device[drive].dev.groups = floppy_dev_groups;

		err = platform_device_register(&floppy_device[drive]);
		if (err)
			goto out_remove_drives;

		/* to be cleaned up... */
		disks[drive]->private_data = (void *)(long)drive;
		disks[drive]->flags |= GENHD_FL_REMOVABLE;
		device_add_disk(&floppy_device[drive].dev, disks[drive]);
	}

	return 0;

out_remove_drives:
	while (drive--) {
		if (floppy_available(drive)) {
			del_gendisk(disks[drive]);
			platform_device_unregister(&floppy_device[drive]);
		}
	}
out_release_dma:
	if (atomic_read(&usage_count))
		floppy_release_irq_and_dma();
out_unreg_region:
	blk_unregister_region(MKDEV(FLOPPY_MAJOR, 0), 256);
	platform_driver_unregister(&floppy_driver);
out_unreg_blkdev:
	unregister_blkdev(FLOPPY_MAJOR, "fd");
out_put_disk:
	destroy_workqueue(floppy_wq);
	for (drive = 0; drive < N_DRIVE; drive++) {
		if (!disks[drive])
			break;
		if (disks[drive]->queue) {
			del_timer_sync(&motor_off_timer[drive]);
			blk_cleanup_queue(disks[drive]->queue);
			disks[drive]->queue = NULL;
		}
		put_disk(disks[drive]);
	}
	return err;
}

#ifndef MODULE
static __init void floppy_async_init(void *data, async_cookie_t cookie)
{
	do_floppy_init();
}
#endif

static int __init floppy_init(void)
{
#ifdef MODULE
	return do_floppy_init();
#else
	/* Don't hold up the bootup by the floppy initialization */
	async_schedule(floppy_async_init, NULL);
	return 0;
#endif
}

static const struct io_region {
	int offset;
	int size;
} io_regions[] = {
	{ 2, 1 },
	/* address + 3 is sometimes reserved by pnp bios for motherboard */
	{ 4, 2 },
	/* address + 6 is reserved, and may be taken by IDE.
	 * Unfortunately, Adaptec doesn't know this :-(, */
	{ 7, 1 },
};

static void floppy_release_allocated_regions(int fdc, const struct io_region *p)
{
	while (p != io_regions) {
		p--;
		release_region(FDCS->address + p->offset, p->size);
	}
}

#define ARRAY_END(X) (&((X)[ARRAY_SIZE(X)]))

static int floppy_request_regions(int fdc)
{
	const struct io_region *p;

	for (p = io_regions; p < ARRAY_END(io_regions); p++) {
		if (!request_region(FDCS->address + p->offset,
				    p->size, "floppy")) {
			DPRINT("Floppy io-port 0x%04lx in use\n",
			       FDCS->address + p->offset);
			floppy_release_allocated_regions(fdc, p);
			return -EBUSY;
		}
	}
	return 0;
}

static void floppy_release_regions(int fdc)
{
	floppy_release_allocated_regions(fdc, ARRAY_END(io_regions));
}

static int floppy_grab_irq_and_dma(void)
{
	if (atomic_inc_return(&usage_count) > 1)
		return 0;

	/*
	 * We might have scheduled a free_irq(), wait it to
	 * drain first:
	 */
	flush_workqueue(floppy_wq);

	if (fd_request_irq()) {
		DPRINT("Unable to grab IRQ%d for the floppy driver\n",
		       FLOPPY_IRQ);
		atomic_dec(&usage_count);
		return -1;
	}
	if (fd_request_dma()) {
		DPRINT("Unable to grab DMA%d for the floppy driver\n",
		       FLOPPY_DMA);
		if (can_use_virtual_dma & 2)
			use_virtual_dma = can_use_virtual_dma = 1;
		if (!(can_use_virtual_dma & 1)) {
			fd_free_irq();
			atomic_dec(&usage_count);
			return -1;
		}
	}

	for (fdc = 0; fdc < N_FDC; fdc++) {
		if (FDCS->address != -1) {
			if (floppy_request_regions(fdc))
				goto cleanup;
		}
	}
	for (fdc = 0; fdc < N_FDC; fdc++) {
		if (FDCS->address != -1) {
			reset_fdc_info(1);
			fd_outb(FDCS->dor, FD_DOR);
		}
	}
	fdc = 0;
	set_dor(0, ~0, 8);	/* avoid immediate interrupt */

	for (fdc = 0; fdc < N_FDC; fdc++)
		if (FDCS->address != -1)
			fd_outb(FDCS->dor, FD_DOR);
	/*
	 * The driver will try and free resources and relies on us
	 * to know if they were allocated or not.
	 */
	fdc = 0;
	irqdma_allocated = 1;
	return 0;
cleanup:
	fd_free_irq();
	fd_free_dma();
	while (--fdc >= 0)
		floppy_release_regions(fdc);
	atomic_dec(&usage_count);
	return -1;
}

static void floppy_release_irq_and_dma(void)
{
	int old_fdc;
#ifndef __sparc__
	int drive;
#endif
	long tmpsize;
	unsigned long tmpaddr;

	if (!atomic_dec_and_test(&usage_count))
		return;

	if (irqdma_allocated) {
		fd_disable_dma();
		fd_free_dma();
		fd_free_irq();
		irqdma_allocated = 0;
	}
	set_dor(0, ~0, 8);
#if N_FDC > 1
	set_dor(1, ~8, 0);
#endif

	if (floppy_track_buffer && max_buffer_sectors) {
		tmpsize = max_buffer_sectors * 1024;
		tmpaddr = (unsigned long)floppy_track_buffer;
		floppy_track_buffer = NULL;
		max_buffer_sectors = 0;
		buffer_min = buffer_max = -1;
		fd_dma_mem_free(tmpaddr, tmpsize);
	}
#ifndef __sparc__
	for (drive = 0; drive < N_FDC * 4; drive++)
		if (timer_pending(motor_off_timer + drive))
			pr_info("motor off timer %d still active\n", drive);
#endif

	if (delayed_work_pending(&fd_timeout))
		pr_info("floppy timer still active:%s\n", timeout_message);
	if (delayed_work_pending(&fd_timer))
		pr_info("auxiliary floppy timer still active\n");
	if (work_pending(&floppy_work))
		pr_info("work still pending\n");
	old_fdc = fdc;
	for (fdc = 0; fdc < N_FDC; fdc++)
		if (FDCS->address != -1)
			floppy_release_regions(fdc);
	fdc = old_fdc;
}

#ifdef MODULE

static char *floppy;

static void __init parse_floppy_cfg_string(char *cfg)
{
	char *ptr;

	while (*cfg) {
		ptr = cfg;
		while (*cfg && *cfg != ' ' && *cfg != '\t')
			cfg++;
		if (*cfg) {
			*cfg = '\0';
			cfg++;
		}
		if (*ptr)
			floppy_setup(ptr);
	}
}

static int __init floppy_module_init(void)
{
	if (floppy)
		parse_floppy_cfg_string(floppy);
	return floppy_init();
}
module_init(floppy_module_init);

static void __exit floppy_module_exit(void)
{
	int drive;

	blk_unregister_region(MKDEV(FLOPPY_MAJOR, 0), 256);
	unregister_blkdev(FLOPPY_MAJOR, "fd");
	platform_driver_unregister(&floppy_driver);

	destroy_workqueue(floppy_wq);

	for (drive = 0; drive < N_DRIVE; drive++) {
		del_timer_sync(&motor_off_timer[drive]);

		if (floppy_available(drive)) {
			del_gendisk(disks[drive]);
			platform_device_unregister(&floppy_device[drive]);
		}
		blk_cleanup_queue(disks[drive]->queue);

		/*
		 * These disks have not called add_disk().  Don't put down
		 * queue reference in put_disk().
		 */
		if (!(allowed_drive_mask & (1 << drive)) ||
		    fdc_state[FDC(drive)].version == FDC_NONE)
			disks[drive]->queue = NULL;

		put_disk(disks[drive]);
	}

	cancel_delayed_work_sync(&fd_timeout);
	cancel_delayed_work_sync(&fd_timer);

	if (atomic_read(&usage_count))
		floppy_release_irq_and_dma();

	/* eject disk, if any */
	fd_eject(0);
}

module_exit(floppy_module_exit);

module_param(floppy, charp, 0);
module_param(FLOPPY_IRQ, int, 0);
module_param(FLOPPY_DMA, int, 0);
MODULE_AUTHOR("Alain L. Knaff");
MODULE_SUPPORTED_DEVICE("fd");
MODULE_LICENSE("GPL");

/* This doesn't actually get used other than for module information */
static const struct pnp_device_id floppy_pnpids[] = {
	{"PNP0700", 0},
	{}
};

MODULE_DEVICE_TABLE(pnp, floppy_pnpids);

#else

__setup("floppy=", floppy_setup);
module_init(floppy_init)
#endif

MODULE_ALIAS_BLOCKDEV_MAJOR(FLOPPY_MAJOR);