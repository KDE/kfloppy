    /*

    $Id$

    Copyright (C) 1997 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   Modified for use with kfloppy.
   I fixed some bugs in bad block marking along the way.
   Bernd Wuebben 

   Here is the original header copyright notice:
   ----------------------------------------------------------------

   Filename:     mkdosfs.c
   Version:      0.3b (Yggdrasil)
   Author:       Dave Hudson
   Started:      24th August 1994
   Last Updated: 5th May 1995
   Updated by:   H. Peter Anvin <hpa@yggdrasil.com>
   Target O/S:   Linux (1.x)

   Description: Utility to allow an MS-DOS filesystem to be created
   under Linux.  A lot of the basic structure of this program has been
   borrowed from Remy Card's "mke2fs" code.

   As far as possible the aim here is to make the "mkdosfs" command
   look almost identical to the other Linux filesystem make utilties,
   eg bad blocks are still specified as blocks, not sectors, but when
   it comes down to it, DOS is tied to the idea of a sector (512 bytes
   as a rule), and not the block.  For example the boot block does not
   occupy a full cluster.

   Copying:     Copyright 1993, 1994 David Hudson (dave@humbug.demon.co.uk)

   Portions copyright 1992, 1993 Remy Card (card@masi.ibp.fr)
   and 1991 Linus Torvalds (torvalds@klaava.helsinki.fi)

   */

#include <fcntl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>

/* this is to get rid of some warnings ... */
#if  !(__GLIBC__ >= 2)  
#include <linux/stat.h>
#endif

#include <linux/fd.h>
#include <mntent.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


/* Constant definitions */

#define TRUE 1			/* Boolean constants */
#define FALSE 0

#define TEST_BUFFER_BLOCKS 16
#define SECTORS_PER_BLOCK ( BLOCK_SIZE / SECTOR_SIZE )


/* Macro definitions */

/* Report a failure message and return a failure error code */

#define die( str ) fatal_error( "%s: " str "\n" )


/* Mark a cluster in the FAT as bad */

#define mark_sector_bad( sector ) mark_FAT_sector( sector, 0xfff7 )

/* Compute ceil(a/b) */

inline int
cdiv (int a, int b)
{
  return (a + b + 1) / b;
}

/* MS-DOS filesystem structures -- I included them here instead of
   including linux/msdos_fs.h since that doesn't include some fields we
   need */

#define SECTOR_SIZE     512 /* sector size (bytes) */

#define ATTR_RO      1		/* read-only */
#define ATTR_HIDDEN  2		/* hidden */
#define ATTR_SYS     4		/* system */
#define ATTR_VOLUME  8		/* volume label */
#define ATTR_DIR     16		/* directory */
#define ATTR_ARCH    32		/* archived */

#define ATTR_NONE    0		/* no attribute bits */
#define ATTR_UNUSED  (ATTR_VOLUME | ATTR_ARCH | ATTR_SYS | ATTR_HIDDEN)
	/* attribute bits that are copied "as is" */

#define MSDOS_EXT_SIGN 0x29	/* extended boot sector signature */
#define MSDOS_FAT12_SIGN "FAT12   "	/* FAT12 filesystem signature */
#define MSDOS_FAT16_SIGN "FAT16   "	/* FAT16 filesystem signature */

#define BOOT_SIGN 0xAA55	/* Boot sector magic number */

struct msdos_boot_sector
{
  unsigned char boot_jump[3];	/* Boot strap short or near jump */
  char system_id[8];		/* Name - can be used to special case
				   partition manager volumes */
  unsigned char sector_size[2];	/* bytes per logical sector */
  unsigned char cluster_size;	/* sectors/cluster */
  unsigned short reserved;	/* reserved sectors */
  unsigned char fats;		/* number of FATs */
  unsigned char dir_entries[2];	/* root directory entries */
  unsigned char sectors[2];	/* number of sectors */
  unsigned char media;		/* media code (unused) */
  unsigned short fat_length;	/* sectors/FAT */
  unsigned short secs_track;	/* sectors per track */
  unsigned short heads;		/* number of heads */
  unsigned long hidden;		/* hidden sectors (unused) */
  unsigned long total_sect;	/* number of sectors (if sectors == 0) */
  unsigned char drive_number;	/* BIOS drive number */
  unsigned char RESERVED;	/* Unused */
  unsigned char ext_boot_sign;	/* 0x29 if fields below exist (DOS 3.3+) */
  unsigned char volume_id[4];	/* Volume ID number */
  char volume_label[11];	/* Volume label */
  char fs_type[8];		/* Typically FAT12 or FAT16 */
  unsigned char boot_code[448];	/* Boot code (or message) */
  unsigned short boot_sign;	/* 0xAA55 */
};

struct msdos_dir_entry
  {
    char name[8], ext[3];	/* name and extension */
    unsigned char attr;		/* attribute bits */
    char unused[10];
    unsigned short time, date, start;	/* time, date and first cluster */
    unsigned long size;		/* file size (in bytes) */
  };

/* The "boot code" we put into the filesystem... it writes a message and
   tells the user to try again */

char dummy_boot_jump[3] = { 0xeb, 0x3c, 0x90 };

char dummy_boot_code[448] =
  "\x0e"			/* push cs */
  "\x1f"			/* pop ds */
  "\xbe\x5b\x7c"		/* mov si, offset message_txt */
				/* write_msg: */
  "\xac"			/* lodsb */
  "\x22\xc0"			/* and al, al */
  "\x74\x0b"			/* jz key_press */
  "\x56"			/* push si */
  "\xb4\x0e"			/* mov ah, 0eh */
  "\xbb\x07\x00"		/* mov bx, 0007h */
  "\xcd\x10"			/* int 10h */
  "\x5e"			/* pop si */
  "\xeb\xf0"			/* jmp write_msg */
				/* key_press: */
  "\x32\xe4"			/* xor ah, ah */
  "\xcd\x16"			/* int 16h */
  "\xcd\x19"			/* int 19h */
  "\xeb\xfe"			/* foo: jmp foo */
				/* message_txt: */

  "This is not a bootable disk.  Please insert a bootable floppy and\r\n"
  "press any key to try again ... \r\n";

#define MESSAGE_OFFSET 29	/* Offset of message in above code */

/* Global variables - the root of all evil :-) - see these and weep! */

static char *program_name = "kmkdosfs";	/* Name of the program */
static char *device_name = NULL;	/* Name of the device on which to create the filesystem */
static int check = FALSE;	/* Default to no readablity checking */
static int verbose = FALSE;	/* Default to verbose mode off */
static long volume_id;		/* Volume ID number */
static time_t create_time;	/* Creation time */
static char volume_name[] = "           "; /* Volume name */
static int blocks;		/* Number of blocks in filesystem */
static int badblocks = 0;	/* Number of bad blocks in the filesystam */
static int nr_fats = 2;		/* Default number of FATs to produce */
static int size_fat = 0;	/* Size in bits of FAT entries */
static int dev = -1;		/* FS block device file handle */
static unsigned int currently_testing = 0;	/* Block currently being tested (if autodetect bad blocks) */
static struct msdos_boot_sector bs;	/* Boot sector data */
static int start_data_sector;	/* Sector number for the start of the data area */
static int start_data_block;	/* Block number for the start of the data area */
static int block_size = BLOCK_SIZE;	/* Size of a block */
static unsigned char *fat;	/* File allocation table */
static struct msdos_dir_entry *root_dir;	/* Root directory */
static int size_root_dir;	/* Size of the root directory in bytes */
static int sectors_per_cluster = 0;	/* Number of sectors per disk cluster */
static int root_dir_entries = 0;	/* Number of root directory entries */
static char blank_sector[SECTOR_SIZE];	/* Blank sector - all zeros */


/* Function prototype definitions */

static volatile void fatal_error (const char *fmt_string);
static void mark_FAT_cluster (int cluster, unsigned int value);
static void mark_FAT_sector (int sector, unsigned int value);
static long do_check (char *buffer, int try, unsigned int current_block);
static void alarm_intr (int alnum);
static void check_blocks (void);
static void get_list_blocks (char *filename);
static int valid_offset (int fd, int offset);
static int count_blocks (char *filename);
static void check_mount (char *device_name);
static void establish_params (int device_num);
static void setup_tables (void);
static void write_tables (void);


/* The function implementations */

/* Handle the reporting of fatal errors.  Volatile to let gcc know that this doesn't return */

static volatile void
fatal_error (const char *fmt_string)
{
  fprintf (stderr, fmt_string, program_name, device_name);
  exit (1);			/* The error exit code is 1! */
}


/* Mark the specified cluster as having a particular value */

static void
mark_FAT_cluster (int cluster, unsigned int value)
{
  if (size_fat == 16)
    {
      value &= 0xffff;
      fat[2 * cluster] = (unsigned char) (value & 0x00ff);
      fat[(2 * cluster) + 1] = (unsigned char) (value >> 8);
    }
  else
    {
      value &= 0x0fff;
      if (((cluster * 3) & 0x1) == 0)
	{
	  fat[3 * cluster / 2] = (unsigned char) (value & 0x00ff);
	  fat[(3 * cluster / 2) + 1] = (unsigned char) ((fat[(3 * cluster / 2) + 1] & 0x00f0)
						 | ((value & 0x0f00) >> 8));
	}
      else
	{
	  fat[3 * cluster / 2] = (unsigned char) ((fat[3 * cluster / 2] & 0x000f) | ((value & 0x000f) << 4));
	  fat[(3 * cluster / 2) + 1] = (unsigned char) ((value & 0x0ff0) >> 4);
	}
    }
}


/* Mark a specified sector as having a particular value in it's FAT entry */

static void
mark_FAT_sector (int sector, unsigned int value)
{
  int cluster;

  cluster = (sector - start_data_sector) / (int) (bs.cluster_size);
  if (cluster < 0)
    die ("Invalid cluster number in mark_FAT_sector: probably bug!");

  mark_FAT_cluster (cluster, value);
}


/* Perform a test on a block.  Return the number of blocks that could be read successfully */

static long
do_check (char *buffer, int try, unsigned int current_block)
{
  long got;

  if (lseek (dev, current_block * block_size, SEEK_SET)		/* Seek to the correct location */
      != current_block * block_size)
    die ("seek failed during testing for blocks");

  got = read (dev, buffer, try * block_size);	/* Try reading! */
  if (got < 0)
    got = 0;

  if (got & (block_size - 1))
    printf ("Unexpected values in do_check: probably bugs\n");
  got /= block_size;

  return got;
}


/* Alarm clock handler - display the status of the quest for bad blocks!  Then retrigger the alarm for five senconds
   later (so we can come here again) */

static void
alarm_intr (int alnum)
{
  if (currently_testing >= blocks)
    return;

  signal (SIGALRM, alarm_intr);
  alarm (5);
  if (!currently_testing)
    return;

  printf ("%d... ", currently_testing);
  fflush (stdout);
}


static void
check_blocks (void)
{
  int try, got;
  int i;
  int nextmsg  = 0;
  static char blkbuf[BLOCK_SIZE * TEST_BUFFER_BLOCKS];

  if (verbose)
    {
      printf ("Searching for bad blocks ");
      fflush (stdout);
    }
  currently_testing = 0;
  if (verbose)
    {
      /*      signal (SIGALRM, alarm_intr);
      alarm (5);*/
    }
  try = TEST_BUFFER_BLOCKS;
  while (currently_testing < blocks)
    {
      if (currently_testing + try > blocks)
	try = blocks - currently_testing;
      got = do_check (blkbuf, try, currently_testing);
      currently_testing += got;

      if(currently_testing >= nextmsg){
	/*	printf("BLOCK %08d %d\n",currently_testing,mycounter++);*/
	printf("BLOCK %08d\n",currently_testing);
	fflush(stdout);
	nextmsg += TEST_BUFFER_BLOCKS;
      }

      if (got == try)
	{
	  try = TEST_BUFFER_BLOCKS;
	  continue;
	}
      else
	try = 1;

      if (currently_testing < start_data_block)
	die ("Bad blocks before data-area: Cannot create a DOS fs.\n"\
	     "Try ext2fs instead or dispose of floppy.\n");

      if(got == 0){

	for (i = 0; i < SECTORS_PER_BLOCK; i++)	
	  mark_sector_bad (currently_testing * SECTORS_PER_BLOCK + i);

	printf("BBF %08d\n",currently_testing);
	fflush(stdout);

	currently_testing ++;
	badblocks++;
      }
    }

  printf ("TNBB %08d\n", badblocks);
  fflush(stdout);
}


static void
get_list_blocks (char *filename)
{
  int i;
  FILE *listfile;
  unsigned long blockno;

  listfile = fopen (filename, "r");
  if (listfile == (FILE *) NULL)
    die ("Can't open file of bad blocks");

  while (!feof (listfile))
    {
      fscanf (listfile, "%ld\n", &blockno);
      for (i = 0; i < SECTORS_PER_BLOCK; i++)	/* Mark all of the sectors in the block as bad */
	mark_sector_bad (blockno * SECTORS_PER_BLOCK + i);
      badblocks++;
    }
  fclose (listfile);

  if (badblocks)
    printf ("%d bad block%s\n", badblocks,
	    (badblocks > 1) ? "s" : "");
}


/* Given a file descriptor and an offset, check whether the offset is a valid offset for the file - return FALSE if it
   isn't valid or TRUE if it is */

static int
valid_offset (int fd, int offset)
{
  char ch;

  if (lseek (fd, offset, 0) < 0)
    return FALSE;
  if (read (fd, &ch, 1) < 1)
    return FALSE;
  return TRUE;
}


/* Given a filename, look to see how many blocks of BLOCK_SIZE are present, returning the answer */

static int
count_blocks (char *filename)
{
  unsigned long high, low;
  int fd;

  if ((fd = open (filename, O_RDONLY)) < 0)
    {
      perror (filename);
      exit (1);
    }
  low = 0;

  for (high = 1; valid_offset (fd, high); high *= 2)
    low = high;
  while (low < high - 1)
    {
      const int mid = (low + high) / 2;

      if (valid_offset (fd, mid))
	low = mid;
      else
	high = mid;
    }
  valid_offset (fd, 0);
  close (fd);

  return (low + 1) / BLOCK_SIZE;
}


/* Check to see if the specified device is currently mounted - abort if it is */

static void
check_mount (char *device_name)
{
  FILE *f;
  struct mntent *mnt;

  if ((f = setmntent (MOUNTED, "r")) == NULL)
    return;
  while ((mnt = getmntent (f)) != NULL)
    if (strcmp (device_name, mnt->mnt_fsname) == 0)
      die ("%s contains a mounted file system.");
  endmntent (f);
}


/* Establish the geometry and media parameters for the device */

static void
establish_params (int device_num)
{
  struct hd_geometry geometry;
  struct floppy_struct param;

  if ((device_num & 0xff00) == 0x0200)	/* Is this a floppy diskette? */
    {
      if (ioctl (dev, FDGETPRM, &param))	/*  Can we get the diskette geometry? */
	die ("unable to get diskette geometry for '%s'");

      bs.secs_track = param.sect;	/*  Set up the geometry information */
      bs.heads = param.head;
      switch (param.size)	/*  Set up the media descriptor byte */
	{
	case 720:		/* 5.25", 2, 9, 40 - 360K */
	  bs.media = (char) 0xfd;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 112;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 1440:		/* 3.5", 2, 9, 80 - 720K */
	  bs.media = (char) 0xf9;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 112;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 2400:		/* 5.25", 2, 15, 80 - 1200K */
	  bs.media = (char) 0xf9;
	  bs.cluster_size = (char) 1;
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;
	  
	case 5760:		/* 3.5", 2, 36, 80 - 2880K */
	  bs.media = (char) 0xf0;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 2880:		/* 3.5", 2, 18, 80 - 1440K */
	default:		/* Anything else */
	  bs.media = (char) 0xf0;
	  bs.cluster_size = (char) 1;
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;
	}
    }
  else
    /* Must be a hard disk then! */
    {
      /* Can we get the drive geometry? (Note I'm not too sure about */
      /* whether to use HDIO_GETGEO or HDIO_REQ) */
      if (ioctl (dev, HDIO_GETGEO, &geometry))
	die ("unable to get drive geometry for '%s'");
      bs.secs_track = geometry.sectors;	/* Set up the geometry information */
      bs.heads = geometry.heads;
      bs.media = (char) 0xf8; /* Set up the media descriptor for a hard drive */
      bs.cluster_size = (char) 4;	/* Start at 4 sectors per cluster */
      bs.dir_entries[0] = (char) 0;	/* Default to 512 entries */
      bs.dir_entries[1] = (char) 2;
    }
}


/* Create the filesystem data tables */

static void
setup_tables (void)
{
  int num_sectors;
  int cluster_count = 0;
  int i;
  int fatlength12, fatlength16;
  int maxclust12, maxclust16;
  int clust12, clust16;
  int fatdata;			/* Sectors for FATs + data area */
  int maxclustsize;
  struct tm *ctime;

  strcpy (bs.system_id, "kmkdosfs");
  if (sectors_per_cluster)
    bs.cluster_size = (char) sectors_per_cluster;
  if (root_dir_entries)
    {
      bs.dir_entries[0] = (char) (root_dir_entries & 0x00ff);
      bs.dir_entries[1] = (char) ((root_dir_entries & 0xff00) >> 8);
    }
  else
    root_dir_entries = bs.dir_entries[0] + (bs.dir_entries[1] << 8);

  bs.volume_id[0] = (unsigned char) (volume_id & 0x000000ff);
  bs.volume_id[1] = (unsigned char) ((volume_id & 0x0000ff00) >> 8);
  bs.volume_id[2] = (unsigned char) ((volume_id & 0x00ff0000) >> 16);
  bs.volume_id[3] = (unsigned char) (volume_id >> 24);

  memcpy(bs.volume_label, volume_name, 11);

  memcpy(bs.boot_jump, dummy_boot_jump, 3);
  memcpy(bs.boot_code, dummy_boot_code, 448);
  bs.boot_sign = BOOT_SIGN;

  bs.sector_size[0] = (char) (SECTOR_SIZE & 0x00ff);
  bs.sector_size[1] = (char) ((SECTOR_SIZE & 0xff00) >> 8);
  bs.reserved = 1;
  bs.fats = (char) nr_fats;
  num_sectors = blocks * SECTORS_PER_BLOCK;
  if (num_sectors >= 65536)
    {
      bs.sectors[0] = (char) 0;
      bs.sectors[1] = (char) 0;
      bs.total_sect = num_sectors;
    }
  else
    {
      bs.sectors[0] = (char) (num_sectors & 0x00ff);
      bs.sectors[1] = (char) ((num_sectors & 0xff00) >> 8);
      bs.total_sect = 0;
    }
  bs.hidden = 0;

  fatdata = num_sectors - cdiv (root_dir_entries * 32, SECTOR_SIZE) -
    bs.reserved;

  if (sectors_per_cluster)
    {
      bs.cluster_size = maxclustsize = sectors_per_cluster;
    }
  else
    {
      /* An initial guess for bs.cluster_size should already be set */
      maxclustsize = 128;
    }

  do
    {
      clust12 = (long long) fatdata *SECTOR_SIZE /
	((int) bs.cluster_size * SECTOR_SIZE + 3);
      fatlength12 = cdiv ((clust12 * 3 + 1) >> 1, SECTOR_SIZE);
      maxclust12 = (fatlength12 * 2 * SECTOR_SIZE) / 3;
      if (maxclust12 > 4086)
	maxclust12 = 4086;
      if (clust12 > maxclust12)
	clust12 = 0;

      clust16 = (long long) fatdata *SECTOR_SIZE /
	((int) bs.cluster_size * SECTOR_SIZE + 4);
      fatlength16 = cdiv (clust16 * 2, SECTOR_SIZE);
      maxclust16 = (fatlength16 * SECTOR_SIZE) / 2;
      if (maxclust16 > 65526)
	maxclust16 = 65526;
      if (clust16 > maxclust16)
	clust16 = 0;

      if ((clust12 && size_fat != 16) || (clust16 && size_fat != 12))
	break;

      bs.cluster_size <<= 1;
    }
  while (bs.cluster_size <= maxclustsize);

  /* Use the optimal FAT size */
  if (!size_fat)
    size_fat = (clust16 > clust12) ? 16 : 12;

  switch (size_fat)
    {
    case 12:
      cluster_count = clust12;
      bs.fat_length = fatlength12;
      memcpy(bs.fs_type, MSDOS_FAT12_SIGN, 8);
      break;

    case 16:
      cluster_count = clust16;
      bs.fat_length = fatlength16;
      memcpy(bs.fs_type, MSDOS_FAT16_SIGN, 8);
      break;

    default:
      die("FAT not 12 or 16 bits\n");
    }

  bs.ext_boot_sign = MSDOS_EXT_SIGN;

  if (!cluster_count)
    {
      if (sectors_per_cluster)	/* If yes, die if we'd spec'd sectors per cluster */
	die ("Too many clusters for file system - try more sectors per cluster");
      else
	die ("Attempting to create too large a file system");
    }

  

  start_data_sector = (int) (bs.reserved) + nr_fats * (int) (bs.fat_length);
  start_data_block = (start_data_sector + SECTORS_PER_BLOCK - 1) / SECTORS_PER_BLOCK;

  if (blocks < start_data_block + 32)	/* Arbitrary undersize file system! */
    die ("Too few blocks available for a viable DOS file system.\n"\
	 "          Try ext2fs instead or dispose of floppy.\n");

  if (verbose)
    {
      printf("%s has %d head%s and %d sector%s per track,\n",
	     device_name, bs.heads, (bs.heads != 1) ? "s" : "",
	     bs.secs_track, (bs.secs_track != 1) ? "s" : ""); 
      printf("using 0x%02x media descriptor, with %d sectors;\n",
	     (int) (bs.media), num_sectors);
      printf("file system has %d %d-bit FAT%s and %d sector%s per cluster.\n",
	     (int) (bs.fats), size_fat, (bs.fats != 1) ? "s" : "",
	     (int) (bs.cluster_size), (bs.cluster_size != 1) ? "s" : "");
      printf ("FAT size is %d sector%s, and provides %d cluster%s.\n",
	      (int) (bs.fat_length), (bs.fat_length != 1) ? "s" : "",
	      cluster_count, (cluster_count != 1) ? "s" : "");
      printf ("Root directory contains %d slots.\n",
	      (int) (bs.dir_entries[0]) + (int) (bs.dir_entries[1]) * 256);
      printf ("Volume ID is %08lx, ", volume_id);
      if ( strcmp(volume_name, "           ") )
	printf("volume label %s.\n", volume_name);
      else
	printf("no volume label.\n");
    }

  /* Make the file allocation tables! */

  if ((fat = (unsigned char *) malloc ((int) (bs.fat_length) * SECTOR_SIZE)) == NULL)
    die ("unable to allocate space for FAT image in memory");

  for (i = 0; i < (int) (bs.fat_length) * SECTOR_SIZE; i++)
    fat[i] = (char) 0;

  mark_FAT_cluster (0, 0xffff);	/* Initial fat entries */
  mark_FAT_cluster (1, 0xffff);
  fat[0] = (unsigned char) bs.media;	/* Put media type in first byte! */

  /* Make the root directory entries */

  size_root_dir = ((int) (bs.dir_entries[1]) * 256
		   + (int) (bs.dir_entries[0]))
    * sizeof (struct msdos_dir_entry);
  if ((root_dir = (struct msdos_dir_entry *) malloc (size_root_dir)) == NULL)
    {
      free (fat);		/* Tidy up before we die! */
      die ("unable to allocate space for root directory in memory");
    }

  if ( !memcmp(volume_name, "           ", 11) )
    {
      memset(root_dir, 0, size_root_dir);
    }
  else
    {
      memcpy(root_dir[0].name, volume_name, 11);
      root_dir[0].attr = ATTR_VOLUME;
      ctime = localtime(&create_time);
      root_dir[0].time = (unsigned short)((ctime->tm_sec >> 1) +
			  (ctime->tm_min << 5) + (ctime->tm_hour << 11));
      root_dir[0].date = (unsigned short)(ctime->tm_mday +
					  ((ctime->tm_mon+1) << 5) +
					  ((ctime->tm_year-80) << 9));
      root_dir[0].start = 0;
      root_dir[0].size = 0;
      memset(root_dir+1, 0, size_root_dir-sizeof(struct msdos_dir_entry));
    }

  memset(blank_sector, 0, SECTOR_SIZE);
}


/* Write the new filesystem's data tables to wherever they're going to end up! */

static void
write_tables (void)
{
  int x;

  if (lseek (dev, 0, SEEK_SET))
    {
      free (fat);
      free (root_dir);
      die ("seek failed whilst writing tables");
    }
  if (write (dev, blank_sector, SECTOR_SIZE) != SECTOR_SIZE)
    {
      free (fat);
      free (root_dir);
      die ("failed whilst blanking boot sector");
    }
  if (lseek (dev, 0, SEEK_SET))
    {
      free (fat);
      free (root_dir);
      die ("seek 2 failed whilst writing tables");
    }
  if (write (dev, (char *) &bs, sizeof (struct msdos_boot_sector)) != sizeof (struct msdos_boot_sector))
    {
      free (fat);
      free (root_dir);
      die ("failed whilst writing boot sector");
    }
  if (lseek (dev, SECTOR_SIZE * (int) (bs.reserved), SEEK_SET) != SECTOR_SIZE * (int) (bs.reserved))
    {
      free (fat);
      free (root_dir);
      die ("seek 3 failed whilst writing tables");
    }
  for (x = 1; x <= nr_fats; x++)
    if (write (dev, fat, (int) (bs.fat_length) * SECTOR_SIZE) != (int) (bs.fat_length) * SECTOR_SIZE)
      {
	free (fat);
	free (root_dir);
	die ("failed whilst writing FAT");
      }
  if (write (dev, (char *) root_dir, size_root_dir) != size_root_dir)
    {
      free (fat);
      free (root_dir);
      die ("failed whilst writing root directory");
    }

  free (root_dir);   /* Free up the root directory space from setup_tables */
  free (fat);  /* Free up the fat table space reserved during setup_tables */
}


/* Report the command usage and return a failure error code */

void
usage (void)
{
  fatal_error("\
Usage: %s [-c | -l bad-block-file] [-m boot-msg-file]\n\
       [-n volume-name] [-i volume-id] [-s sectors-per-cluster] [-v]\n\
       [-f number-of-FATs] [-F fat-size] [-r root-dir-entries]\n\
       /dev/name [blocks]\n");
}

/* The "main" entry point into the utility - we pick up the options and attempt to process them in some sort of sensible
   way.  In the event that some/all of the options are invalid we need to tell the user so that something can be done! */

int
main (int argc, char **argv)
{
  char c;
  char *tmp;
  char *listfile = NULL;
  FILE *msgfile;
  struct stat statbuf;
  int i, pos, ch;

  if (argc && *argv)		/* What's the program name? */
    program_name = *argv;

  time(&create_time);
  volume_id = (long)create_time;	/* Default volume ID = creation time */

  /*  fprintf (stdout, "%s 0.3b (Yggdrasil), 5th May 1995 for MS-DOS FS\n",
	   program_name);*/

  fprintf (stdout, "START\n");
  fflush(stdout);
  while ((c = getopt (argc, argv, "cf:F:i:l:m:n:r:s:v")) != EOF)
    /* Scan the command line for options */
    switch (c)
      {
      case 'c':		/* c : Check FS as we build it */
	check = TRUE;
	break;

      case 'f':		/* f : Choose number of FATs */
	nr_fats = (int) strtol (optarg, &tmp, 0);
	if (*tmp || nr_fats < 1 || nr_fats > 4)
	  {
	    printf ("Bad number of FATs : %s\n", optarg);
	    usage ();
	  }
	break;

      case 'F':		/* F : Choose FAT size */
	size_fat = (int) strtol (optarg, &tmp, 0);
	if (*tmp || (size_fat != 12 && size_fat != 16))
	  {
	    printf ("Bad FAT type : %s\n", optarg);
	    usage ();
	  }
	break;

      case 'i':		/* i : specify volume ID */
	volume_id = strtol(optarg, &tmp, 16);
	if ( *tmp )
	  {
	    printf("Volume ID must be a hexadecimal number\n");
	    usage();
	  }
	break;

      case 'l':		/* l : Bad block filename */
	listfile = optarg;
	break;

      case 'm':		/* m : Set boot message */
	if ( strcmp(optarg, "-") )
	  {
	    msgfile = fopen(optarg, "r");
	    if ( !msgfile )
	      perror(optarg);
	  }
	else
	  msgfile = stdin;

	if ( msgfile )
	  {
	    /* The boot code ends at offset 448 and needs a null terminator */
	    i = MESSAGE_OFFSET;
	    pos = 0;		/* We are at beginning of line */
	    do
	      {
		ch = getc(msgfile);
		switch (ch)
		  {
		  case '\r':	/* Ignore CRs */
		  case '\0':	/* and nulls */
		    break;

		  case '\n':	/* LF -> CR+LF if necessary */
		    if ( pos )	/* If not at beginning of line */
		      {
			dummy_boot_code[i++] = '\r';
			pos = 0;
		      }
		    dummy_boot_code[i++] = '\n';
		    break;

		  case '\t':	/* Expand tabs */
		    do
		      {
			dummy_boot_code[i++] = ' ';
			pos++;
		      }
		    while ( pos % 8 && i < 447 );
		    break;

		  case EOF:
		    dummy_boot_code[i++] = '\0'; /* Null terminator */
		    break;

		  default:
		    dummy_boot_code[i++] = ch; /* Store character */
		    pos++;	/* Advance position */
		    break;
		  }
	      }
	    while ( ch != EOF && i < 447 );
	    
	    dummy_boot_code[447] = '\0'; /* Just in case */
	    
	    if ( ch != EOF )
	      printf ("Warning: message too long; truncated\n");
	    
	    if ( msgfile != stdin )
	      fclose(msgfile);
	  }
	break;

      case 'n':		/* n : Volume name */
	sprintf(volume_name, "%-11.11s", optarg);
	break;

      case 'r':		/* r : Root directory entries */
	root_dir_entries = (int) strtol (optarg, &tmp, 0);
	if (*tmp || root_dir_entries < 16 || root_dir_entries > 32768)
	  {
	    printf ("Bad number of root directory entries : %s\n", optarg);
	    usage ();
	  }
	break;

      case 's':		/* s : Sectors per cluster */
	sectors_per_cluster = (int) strtol (optarg, &tmp, 0);
	if (*tmp || (sectors_per_cluster != 1 && sectors_per_cluster != 2
		     && sectors_per_cluster != 4 && sectors_per_cluster != 8
		   && sectors_per_cluster != 16 && sectors_per_cluster != 32
		&& sectors_per_cluster != 64 && sectors_per_cluster != 128))
	  {
	    printf ("Bad number of sectors per cluster : %s\n", optarg);
	    usage ();
	  }
	break;

      case 'v':		/* v : Verbose execution */
	verbose = TRUE;
	break;
	
      default:
	usage ();
      }

  device_name = argv[optind];	/* Determine the number of blocks in the FS */
  if (optind == argc - 2)	/*  Either check the user specified number */
    blocks = (int) strtol (argv[optind + 1], &tmp, 0);
  else if (optind == argc - 1)	/*  Or have a look and see! */
    {
      blocks = count_blocks (device_name);
      tmp = "";
    }
  else
    usage ();
  if (*tmp)
    {
      printf ("Bad block count : %s\n", argv[optind + 1]);
      usage ();
    }

  if (check && listfile)	/* Auto and specified bad block handling are mutually */
    die ("-c and -l are incompatible");		/* exclusive of each other! */

  check_mount (device_name);	/* Is the device already mounted? */

  dev = open (device_name, O_RDWR);	/* Is it a suitable device to build the FS on? */
  if (dev < 0)
    die ("unable to open %s");
  if (fstat (dev, &statbuf) < 0)
    die ("unable to stat %s");
  if (!S_ISBLK (statbuf.st_mode))
    check = 0;
  else
    /* Ignore any 'full' fixed disk devices */
    if ((statbuf.st_rdev & 0xff3f) == 0x0300 || /* hda, hdb */
	(statbuf.st_rdev & 0xff0f) == 0x0800 || /* sd */
	(statbuf.st_rdev & 0xff3f) == 0x0d00 || /* xd */
	(statbuf.st_rdev & 0xff3f) == 0x1600 )  /* hdc, hdd */
      die ("Will not try to make filesystem on '%s'");

  establish_params (statbuf.st_rdev);	/* Establish the media parameters */
  setup_tables ();		/* Establish the file system tables */

  if (check)			/* Determine any bad block locations and mark them */
    check_blocks ();
  else if (listfile)
    get_list_blocks (listfile);

  write_tables ();		/* Write the file system tables away! */
  fprintf (stdout, "EXIT\n");
  fflush(stdout);
  exit (0);			/* Terminate with no errors! */
}


/* That's All Folks */
