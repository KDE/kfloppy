/*
 * ext2fs.h --- ext2fs
 * 
 * Copyright (C) 1993 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 */

/*
 * Where the master copy of the superblock is located, and how big
 * superblocks are supposed to be.  We define SUPERBLOCK_SIZE because
 * the size of the superblock structure is not necessarily trustworthy
 * (some versions have the padding set up so that the superblock is
 * 1032 bytes long).
 */
#define SUPERBLOCK_OFFSET	1024
#define SUPERBLOCK_SIZE 	1024

/*
 * The last ext2fs revision level that this version of the library is
 * able to support.
 */
#define EXT2_LIB_CURRENT_REV	0

#include <sys/types.h>
#include <linux/types.h>

typedef __u32		blk_t;
typedef unsigned int	dgrp_t;

#include "et/com_err.h"
#include "ext2fs/io.h"
#include "ext2fs/ext2_err.h"

typedef struct struct_ext2_filsys *ext2_filsys;

struct ext2fs_struct_generic_bitmap {
	int		magic;
	ext2_filsys 	fs;
	__u32		start, end;
	__u32		real_end;
	char	*	description;
	char	*	bitmap;
	errcode_t	base_error_code;
	__u32		reserved[7];
};

#define EXT2FS_MARK_ERROR 	0
#define EXT2FS_UNMARK_ERROR 	1
#define EXT2FS_TEST_ERROR	2

typedef struct ext2fs_struct_generic_bitmap *ext2fs_generic_bitmap;

typedef struct ext2fs_struct_generic_bitmap *ext2fs_inode_bitmap;

typedef struct ext2fs_struct_generic_bitmap *ext2fs_block_bitmap;

#ifdef EXT2_DYNAMIC_REV
#define EXT2_FIRST_INODE(s)	EXT2_FIRST_INO(s)
#else
#define EXT2_FIRST_INODE(s)	EXT2_FIRST_INO
#define EXT2_INODE_SIZE(s)	sizeof(struct ext2_inode)
#endif

/*
 * Flags for the ext2_filsys structure
 */

#define EXT2_FLAG_RW		0x01
#define EXT2_FLAG_CHANGED	0x02
#define EXT2_FLAG_DIRTY		0x04
#define EXT2_FLAG_VALID		0x08
#define EXT2_FLAG_IB_DIRTY	0x10
#define EXT2_FLAG_BB_DIRTY	0x20
#define EXT2_FLAG_SWAP_BYTES		0x40
#define EXT2_FLAG_SWAP_BYTES_READ	0x80
#define EXT2_FLAG_SWAP_BYTES_WRITE	0x100
#define EXT2_FLAG_MASTER_SB_ONLY	0x200

/*
 * Special flag in the ext2 inode i_flag field that means that this is
 * a new inode.  (So that ext2_write_inode() can clear extra fields.)
 */
#define EXT2_NEW_INODE_FL	0x80000000

struct struct_ext2_filsys {
	int				magic;
	io_channel			io;
	int				flags;
	char *				device_name;
	struct ext2_super_block	* 	super;
	int				blocksize;
	int				fragsize;
	unsigned long			group_desc_count;
	unsigned long			desc_blocks;
	struct ext2_group_desc *	group_desc;
	int				inode_blocks_per_group;
	ext2fs_inode_bitmap		inode_map;
	ext2fs_block_bitmap		block_map;
	errcode_t (*get_blocks)(ext2_filsys fs, ino_t ino, blk_t *blocks);
	errcode_t (*check_directory)(ext2_filsys fs, ino_t ino);
	errcode_t (*write_bitmaps)(ext2_filsys fs);
	errcode_t (*read_inode)(ext2_filsys fs, ino_t ino,
				struct ext2_inode *inode);
	errcode_t (*write_inode)(ext2_filsys fs, ino_t ino,
				struct ext2_inode *inode);
	__u32				reserved[14];

	/*
	 * Not used by ext2fs library; reserved for the use of the
	 * calling application.
	 */
	void *				private; 
};

/*
 * badblocks list definitions
 */

typedef struct struct_badblocks_list *badblocks_list;

struct struct_badblocks_list {
	int	magic;
	int	num;
	int	size;
	blk_t	*list;
	int	badblocks_flags;
	int	reserved[8];
};

#define BADBLOCKS_FLAG_DIRTY	1

typedef struct struct_badblocks_iterate *badblocks_iterate;

struct struct_badblocks_iterate {
	int		magic;
	badblocks_list	bb;
	int		ptr;
	int	reserved[8];
};

#include "ext2fs/bitops.h"
	
/*
 * Return flags for the block iterator functions
 */
#define BLOCK_CHANGED	1
#define BLOCK_ABORT	2
#define BLOCK_ERROR	4

/*
 * Block interate flags
 *
 * BLOCK_FLAG_APPEND, or BLOCK_FLAG_HOLE, indicates that the interator
 * function should be called on blocks where the block number is zero.
 * This is used by ext2fs_expand_dir() to be able to add a new block
 * to an inode.  It can also be used for programs that want to be able
 * to deal with files that contain "holes".
 * 
 * BLOCK_FLAG_TRAVERSE indicates that the iterator function for the
 * indirect, doubly indirect, etc. blocks should be called after all
 * of the blocks containined in the indirect blocks are processed.
 * This is useful if you are going to be deallocating blocks from an
 * inode.
 *
 * BLOCK_FLAG_DATA_ONLY indicates that the iterator function should be
 * called for data blocks only.
 */
#define BLOCK_FLAG_APPEND	1
#define BLOCK_FLAG_HOLE		1
#define BLOCK_FLAG_DEPTH_TRAVERSE	2
#define BLOCK_FLAG_DATA_ONLY	4

/*
 * Magic "block count" return values for the block iterator function.
 */
#define BLOCK_COUNT_IND		(-1)
#define BLOCK_COUNT_DIND	(-2)
#define BLOCK_COUNT_TIND	(-3)
#define BLOCK_COUNT_TRANSLATOR	(-4)

/*
 * Return flags for the directory iterator functions
 */
#define DIRENT_CHANGED	1
#define DIRENT_ABORT	2
#define DIRENT_ERROR	3

/*
 * Directory iterator flags
 */

#define DIRENT_FLAG_INCLUDE_EMPTY	1

/*
 * Inode scan definitions
 */
typedef struct ext2_struct_inode_scan *ext2_inode_scan;

struct ext2_struct_inode_scan {
	int			magic;
	ext2_filsys		fs;
	ino_t			current_inode;
	blk_t			current_block;
	dgrp_t			current_group;
	int			inodes_left, blocks_left, groups_left;
	int			inode_buffer_blocks;
	char *			inode_buffer;
	int			inode_size;
	char *			ptr;
	int			bytes_left;
	char			*temp_buffer;
	errcode_t		(*done_group)(ext2_filsys fs,
					      ext2_inode_scan scan,
					      dgrp_t group,
					      void * private);
	void *			done_group_data;
	int			reserved[8];
};

/*
 * ext2fs_check_if_mounted flags
 */
#define EXT2_MF_MOUNTED		1
#define EXT2_MF_ISROOT		2
#define EXT2_MF_READONLY	4 

/*
 * Ext2/linux mode flags.  We define them here so that we don't need
 * to depend on the OS's sys/stat.h, since we may be compiling on a
 * non-Linux system.
 */
#define LINUX_S_IFMT  00170000
#define LINUX_S_IFSOCK 0140000
#define LINUX_S_IFLNK	 0120000
#define LINUX_S_IFREG  0100000
#define LINUX_S_IFBLK  0060000
#define LINUX_S_IFDIR  0040000
#define LINUX_S_IFCHR  0020000
#define LINUX_S_IFIFO  0010000
#define LINUX_S_ISUID  0004000
#define LINUX_S_ISGID  0002000
#define LINUX_S_ISVTX  0001000

#define LINUX_S_IRWXU 00700
#define LINUX_S_IRUSR 00400
#define LINUX_S_IWUSR 00200
#define LINUX_S_IXUSR 00100

#define LINUX_S_IRWXG 00070
#define LINUX_S_IRGRP 00040
#define LINUX_S_IWGRP 00020
#define LINUX_S_IXGRP 00010

#define LINUX_S_IRWXO 00007
#define LINUX_S_IROTH 00004
#define LINUX_S_IWOTH 00002
#define LINUX_S_IXOTH 00001

#define LINUX_S_ISLNK(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFLNK)
#define LINUX_S_ISREG(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFREG)
#define LINUX_S_ISDIR(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFDIR)
#define LINUX_S_ISCHR(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFCHR)
#define LINUX_S_ISBLK(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFBLK)
#define LINUX_S_ISFIFO(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFIFO)
#define LINUX_S_ISSOCK(m)	(((m) & LINUX_S_IFMT) == LINUX_S_IFSOCK)

/*
 * For checking structure magic numbers...
 */

#define EXT2_CHECK_MAGIC(struct, code) \
	  if ((struct)->magic != (code)) return (code)


/*
 * The ext2fs library private definition of the ext2 superblock, so we
 * don't have to depend on the kernel's definition of the superblock,
 * which might not have the latest features.
 */
struct ext2fs_sb {
	__u32	s_inodes_count;		/* Inodes count */
	__u32	s_blocks_count;		/* Blocks count */
	__u32	s_r_blocks_count;	/* Reserved blocks count */
	__u32	s_free_blocks_count;	/* Free blocks count */
	__u32	s_free_inodes_count;	/* Free inodes count */
	__u32	s_first_data_block;	/* First Data Block */
	__u32	s_log_block_size;	/* Block size */
	__s32	s_log_frag_size;	/* Fragment size */
	__u32	s_blocks_per_group;	/* # Blocks per group */
	__u32	s_frags_per_group;	/* # Fragments per group */
	__u32	s_inodes_per_group;	/* # Inodes per group */
	__u32	s_mtime;		/* Mount time */
	__u32	s_wtime;		/* Write time */
	__u16	s_mnt_count;		/* Mount count */
	__s16	s_max_mnt_count;	/* Maximal mount count */
	__u16	s_magic;		/* Magic signature */
	__u16	s_state;		/* File system state */
	__u16	s_errors;		/* Behaviour when detecting errors */
	__u16	s_minor_rev_level; 	/* minor revision level */
	__u32	s_lastcheck;		/* time of last check */
	__u32	s_checkinterval;	/* max. time between checks */
	__u32	s_creator_os;		/* OS */
	__u32	s_rev_level;		/* Revision level */
	__u16	s_def_resuid;		/* Default uid for reserved blocks */
	__u16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	__u32	s_first_ino; 		/* First non-reserved inode */
	__u16   s_inode_size; 		/* size of inode structure */
	__u16	s_block_group_nr; 	/* block group # of this superblock */
	__u32	s_feature_compat; 	/* compatible feature set */
	__u32	s_feature_incompat; 	/* incompatible feature set */
	__u32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	__u8	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	__u32	s_reserved[206];	/* Padding to the end of the block */
};
  
/*
 * function prototypes
 */

/* alloc.c */
extern errcode_t ext2fs_new_inode(ext2_filsys fs, ino_t dir, int mode,
				  ext2fs_inode_bitmap map, ino_t *ret);
extern errcode_t ext2fs_new_block(ext2_filsys fs, blk_t goal,
				  ext2fs_block_bitmap map, blk_t *ret);
extern errcode_t ext2fs_get_free_blocks(ext2_filsys fs, blk_t start,
					blk_t finish, int num,
					ext2fs_block_bitmap map,
					blk_t *ret);

/* badblocks.c */
extern errcode_t badblocks_list_create(badblocks_list *ret, int size);
extern void badblocks_list_free(badblocks_list bb);
extern errcode_t badblocks_list_add(badblocks_list bb, blk_t blk);
extern int badblocks_list_test(badblocks_list bb, blk_t blk);
extern errcode_t badblocks_list_iterate_begin(badblocks_list bb,
					      badblocks_iterate *ret);
extern int badblocks_list_iterate(badblocks_iterate iter, blk_t *blk);
extern void badblocks_list_iterate_end(badblocks_iterate iter);

/* bb_inode.c */
extern errcode_t ext2fs_update_bb_inode(ext2_filsys fs,
					badblocks_list bb_list);

/* bitmaps.c */
extern errcode_t ext2fs_write_inode_bitmap(ext2_filsys fs);
extern errcode_t ext2fs_write_block_bitmap (ext2_filsys fs);
extern errcode_t ext2fs_read_inode_bitmap (ext2_filsys fs);
extern errcode_t ext2fs_read_block_bitmap(ext2_filsys fs);
extern errcode_t ext2fs_allocate_generic_bitmap(__u32 start,
						__u32 end,
						__u32 real_end,
						const char *descr,
						ext2fs_generic_bitmap *ret);
extern errcode_t ext2fs_allocate_block_bitmap(ext2_filsys fs,
					      const char *descr,
					      ext2fs_block_bitmap *ret);
extern errcode_t ext2fs_allocate_inode_bitmap(ext2_filsys fs,
					      const char *descr,
					      ext2fs_inode_bitmap *ret);
extern errcode_t ext2fs_fudge_inode_bitmap_end(ext2fs_inode_bitmap bitmap,
					       ino_t end, ino_t *oend);
extern errcode_t ext2fs_fudge_block_bitmap_end(ext2fs_block_bitmap bitmap,
					       blk_t end, blk_t *oend);
extern void ext2fs_clear_inode_bitmap(ext2fs_inode_bitmap bitmap);
extern void ext2fs_clear_block_bitmap(ext2fs_block_bitmap bitmap);
extern errcode_t ext2fs_read_bitmaps(ext2_filsys fs);
extern errcode_t ext2fs_write_bitmaps(ext2_filsys fs);

/* block.c */
extern errcode_t ext2fs_block_iterate(ext2_filsys fs,
				      ino_t	ino,
				      int	flags,
				      char *block_buf,
				      int (*func)(ext2_filsys fs,
						  blk_t	*blocknr,
						  int	blockcnt,
						  void	*private),
				      void *private);

/* check_desc.c */
extern errcode_t ext2fs_check_desc(ext2_filsys fs);

/* closefs.c */
extern errcode_t ext2fs_close(ext2_filsys fs);
extern errcode_t ext2fs_flush(ext2_filsys fs);

/* cmp_bitmaps.c */
extern errcode_t ext2fs_compare_block_bitmap(ext2fs_block_bitmap bm1,
					     ext2fs_block_bitmap bm2);
extern errcode_t ext2fs_compare_inode_bitmap(ext2fs_inode_bitmap bm1,
					     ext2fs_inode_bitmap bm2);


/* dirblock.c */
extern errcode_t ext2fs_read_dir_block(ext2_filsys fs, blk_t block,
				       void *buf);
extern errcode_t ext2fs_write_dir_block(ext2_filsys fs, blk_t block,
					void *buf);

/* expanddir.c */
extern errcode_t ext2fs_expand_dir(ext2_filsys fs, ino_t dir);

/* freefs.c */
extern void ext2fs_free(ext2_filsys fs);
extern void ext2fs_free_generic_bitmap(ext2fs_inode_bitmap bitmap);
extern void ext2fs_free_block_bitmap(ext2fs_block_bitmap bitmap);
extern void ext2fs_free_inode_bitmap(ext2fs_inode_bitmap bitmap);

/* getsize.c */
extern errcode_t ext2fs_get_device_size(const char *file, int blocksize,
					blk_t *retblocks);

/* initialize.c */
extern errcode_t ext2fs_initialize(const char *name, int flags,
				   struct ext2_super_block *param,
				   io_manager manager, ext2_filsys *ret_fs);

/* inode.c */
extern errcode_t ext2fs_open_inode_scan(ext2_filsys fs, int buffer_blocks,
				  ext2_inode_scan *ret_scan);
extern void ext2fs_close_inode_scan(ext2_inode_scan scan);
extern errcode_t ext2fs_get_next_inode(ext2_inode_scan scan, ino_t *ino,
			       struct ext2_inode *inode);
void ext2fs_set_inode_callback(ext2_inode_scan scan,
			       errcode_t (*done_group)(ext2_filsys fs,
						       ext2_inode_scan scan,
						       dgrp_t group,
						       void * private),
			       void *done_group_data);
void ext2fs_set_inode_callback(ext2_inode_scan scan,
			       errcode_t (*done_group)(ext2_filsys fs,
						       ext2_inode_scan scan,
						       dgrp_t group,
						       void * private),
			       void *done_group_data);
extern errcode_t ext2fs_read_inode (ext2_filsys fs, unsigned long ino,
			    struct ext2_inode * inode);
extern errcode_t ext2fs_write_inode(ext2_filsys fs, unsigned long ino,
			    struct ext2_inode * inode);
extern errcode_t ext2fs_get_blocks(ext2_filsys fs, ino_t ino, blk_t *blocks);
extern errcode_t ext2fs_check_directory(ext2_filsys fs, ino_t ino);

/* ismounted.c */
extern errcode_t ext2fs_check_if_mounted(const char *file, int *mount_flags);

/* namei.c */
extern errcode_t ext2fs_dir_iterate(ext2_filsys fs, 
			      ino_t dir,
			      int flags,
			      char *block_buf,
			      int (*func)(struct ext2_dir_entry *dirent,
					  int	offset,
					  int	blocksize,
					  char	*buf,
					  void	*private),
			      void *private);
extern errcode_t ext2fs_lookup(ext2_filsys fs, ino_t dir, const char *name,
			 int namelen, char *buf, ino_t *inode);
extern errcode_t ext2fs_namei(ext2_filsys fs, ino_t root, ino_t cwd,
			const char *name, ino_t *inode);
errcode_t ext2fs_namei_follow(ext2_filsys fs, ino_t root, ino_t cwd,
			      const char *name, ino_t *inode);
extern errcode_t ext2fs_follow_link(ext2_filsys fs, ino_t root, ino_t cwd,
			ino_t inode, ino_t *res_inode);

/* native.c */
int ext2fs_native_flag(void);

/* newdir.c */
extern errcode_t ext2fs_new_dir_block(ext2_filsys fs, ino_t dir_ino,
				ino_t parent_ino, char **block);

/* mkdir.c */
extern errcode_t ext2fs_mkdir(ext2_filsys fs, ino_t parent, ino_t inum,
			      const char *name);

/* openfs.c */
extern errcode_t ext2fs_open(const char *name, int flags, int superblock,
			     int block_size, io_manager manager,
			     ext2_filsys *ret_fs);

/* get_pathname.c */
extern errcode_t ext2fs_get_pathname(ext2_filsys fs, ino_t dir, ino_t ino,
			       char **name);

/* link.c */
errcode_t ext2fs_link(ext2_filsys fs, ino_t dir, const char *name,
		      ino_t ino, int flags);
errcode_t ext2fs_unlink(ext2_filsys fs, ino_t dir, const char *name,
			ino_t ino, int flags);

/* read_bb.c */
extern errcode_t ext2fs_read_bb_inode(ext2_filsys fs, badblocks_list *bb_list);

/* read_bb_file.c */
extern errcode_t ext2fs_read_bb_FILE(ext2_filsys fs, FILE *f, 
				     badblocks_list *bb_list,
				     void (*invalid)(ext2_filsys fs,
						     blk_t blk));

/* swapfs.c */
extern void ext2fs_swap_super(struct ext2_super_block * super);
extern void ext2fs_swap_group_desc(struct ext2_group_desc *gdp);
extern void ext2fs_swap_inode(ext2_filsys fs,struct ext2_inode *t,
			      struct ext2_inode *f, int hostorder);


/* inline functions */
extern void ext2fs_mark_super_dirty(ext2_filsys fs);
extern void ext2fs_mark_changed(ext2_filsys fs);
extern int ext2fs_test_changed(ext2_filsys fs);
extern void ext2fs_mark_valid(ext2_filsys fs);
extern void ext2fs_unmark_valid(ext2_filsys fs);
extern int ext2fs_test_valid(ext2_filsys fs);
extern void ext2fs_mark_ib_dirty(ext2_filsys fs);
extern void ext2fs_mark_bb_dirty(ext2_filsys fs);
extern int ext2fs_test_ib_dirty(ext2_filsys fs);
extern int ext2fs_test_bb_dirty(ext2_filsys fs);
extern int ext2fs_group_of_blk(ext2_filsys fs, blk_t blk);
extern int ext2fs_group_of_ino(ext2_filsys fs, ino_t ino);

/*
 * The actual inlined functions definitions themselves...
 *
 * If NO_INLINE_FUNCS is defined, then we won't try to do inline
 * functions at all!
 */
#if (defined(INCLUDE_INLINE_FUNCS) || !defined(NO_INLINE_FUNCS))
#ifdef INCLUDE_INLINE_FUNCS
#define _INLINE_ extern
#else
#define _INLINE_ extern __inline__
#endif

/*
 * Mark a filesystem superblock as dirty
 */
_INLINE_ void ext2fs_mark_super_dirty(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_DIRTY | EXT2_FLAG_CHANGED;
}

/*
 * Mark a filesystem as changed
 */
_INLINE_ void ext2fs_mark_changed(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_CHANGED;
}

/*
 * Check to see if a filesystem has changed
 */
_INLINE_ int ext2fs_test_changed(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_CHANGED);
}

/*
 * Mark a filesystem as valid
 */
_INLINE_ void ext2fs_mark_valid(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_VALID;
}

/*
 * Mark a filesystem as NOT valid
 */
_INLINE_ void ext2fs_unmark_valid(ext2_filsys fs)
{
	fs->flags &= ~EXT2_FLAG_VALID;
}

/*
 * Check to see if a filesystem is valid
 */
_INLINE_ int ext2fs_test_valid(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_VALID);
}

/*
 * Mark the inode bitmap as dirty
 */
_INLINE_ void ext2fs_mark_ib_dirty(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_IB_DIRTY | EXT2_FLAG_CHANGED;
}

/*
 * Mark the block bitmap as dirty
 */
_INLINE_ void ext2fs_mark_bb_dirty(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_BB_DIRTY | EXT2_FLAG_CHANGED;
}

/*
 * Check to see if a filesystem's inode bitmap is dirty
 */
_INLINE_ int ext2fs_test_ib_dirty(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_IB_DIRTY);
}

/*
 * Check to see if a filesystem's block bitmap is dirty
 */
_INLINE_ int ext2fs_test_bb_dirty(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_BB_DIRTY);
}

/*
 * Return the group # of a block
 */
_INLINE_ int ext2fs_group_of_blk(ext2_filsys fs, blk_t blk)
{
	return (blk - fs->super->s_first_data_block) /
		fs->super->s_blocks_per_group;
}

/*
 * Return the group # of an inode number
 */
_INLINE_ int ext2fs_group_of_ino(ext2_filsys fs, ino_t ino)
{
	return (ino - 1) / fs->super->s_inodes_per_group;
}
#undef _INLINE_
#endif

