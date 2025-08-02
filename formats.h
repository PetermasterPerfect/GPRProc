struct SsTraceHdrStruct
{ /* trace identification header */
	float trace_num; /* trace sequence number within line */
	float position; /* position along traverse */
	float num_samples; /* samples per trace */
	float elevation; /* elevation data if available */
	float unass5; /* unassigned */
	float num_bytes; /* bytes per sample; always 2 for Rev. 3 firmware */
	float aux_trace_num; /* trace number again */
	float num_stacks; /* number of stacks used to get trace */
	float time_window; /* time window in ns */
	float unass10; /* unassigned */
	float unass11; /* unassigned */
	float unass12; /* unassigned */
	float unass13; /* unassigned */
	float unass14; /* unassigned */
	float unass15; /* unassigned */
	float unass16; /* unassigned */
	float unass17; /* unassigned */
	float unass18; /* unassigned */
	float unass19; /* unassigned */
	float unass20; /* unassigned */
	float time_zero_adjust; /* where sample x = sample x + adjustment */
	float zero_flag; /* 0 = data OK, 1 = zero data */
	float unass23; /* unassigned */
	float time; /* seconds past midnight */
	float comment_flag; /* 1 = comment attached */
	char comment[28]; /* optional character comments */
}; 


/* Next two structures are equivalent to GSSI's struct rfdate */
struct __attribute__((packed)) DztDateStruct
{   unsigned sec2  : 5;      /* second/2  (0-29) */
	unsigned min   : 6;      /* minute    (0-59) */
	unsigned hour  : 5;      /* hour      (0-23) */
	unsigned day   : 5;      /* day       (1-31) */
	unsigned month : 4;      /* month     (1-12; 1=Jan, 2=Feb, etc. */
	unsigned year  : 7;      /* year-1980 (0-127; 1980-2107) */
} ;  /* 4 bytes (32 bits) if tightly packed */

struct __attribute__((packed)) TimeDateStruct
{   unsigned sec2  : 5;      /* second/2  (0-29) */
	unsigned min   : 6;      /* minute    (0-59) */
	unsigned hour  : 5;      /* hour      (0-23) */
	unsigned day   : 5;      /* day       (1-31) */
	unsigned month : 4;      /* month     (1-12; 1=Jan, 2=Feb, etc. */
	unsigned year  : 7;      /* year-1980 (0-127; 1980-2107) */
} ; /* 4 bytes (32 bits) if tightly packed */

/* Note: the following structure incorporates the changes noted in the
 *       preliminary draft for extended RADAN file header format as
 *       faxed to Jeff Lucius by Leo Galinovsky (GSSI) 1/11/94.
 * This is the header that starts each DZT file (SIR-10 and SIR-2)
 */
struct __attribute__((packed)) DztHdrStruct
{   unsigned short       rh_tag;       /* 0x0Nff, where N = rh_nchan-1 (0 - 15) */
	unsigned short       rh_data;      /* offset to data in file (1024 * rh_nchan) */
	unsigned short       rh_nsamp;     /* samples per scan (2 - 65535) */
	unsigned short       rh_bits;      /* bits per data word (8, 16, 32, 64) */
	short                rh_zero;      /* binary offset (-128, -32768, etc.) */
	float                rh_sps;       /* scans per second */
	float                rh_spm;       /* scans per meter */
	float                rh_mpm;       /* meters per mark */
	float                rh_position;  /* position (ns) */
	float                rh_range;     /* range (ns) */
	unsigned short       rh_npass;     /* scans per pass for 2D files */
	struct DztDateStruct rh_create;    /* date created */
	struct DztDateStruct rh_modif;     /* date modified */
	unsigned short       rh_rgain;     /* offset to range gain function */
	unsigned short       rh_nrgain;    /* size of range gain function (2 - rh_nsamp) */
	unsigned short       rh_text;      /* offset to text */
	unsigned short       rh_ntext;     /* size of text */
	unsigned short       rh_proc;      /* offset to processing history */
	unsigned short       rh_nproc;     /* size of processing history */
	unsigned short       rh_nchan;     /* number of channels */
	float                rh_epsr;      /* average dielectric constant */
	float                rh_top;       /* top position in meters */
	float                rh_depth;     /* range in meters */
	char                 reserved[31];
	char                 rh_dtype;     /* bits: 7 6 5 4 3 2 1 0
															  0  unsigned data
															  1    signed data
															0     8-bit int
															1    16-bit int
														  1      32-bit int
														1        64-bit int
													  1          32-bit float
													1            64-bit float (USGS extension)
                                                X X X            not used
                                        */
    char                 rh_antname[14]; /* antenna name (eg. 3105 (300 MHz) ) */
    unsigned short       rh_chanmask;    /* active channels mask
                         format is 0x530X, where X has following bits set:
                                          bits: 7 6 5 4 3 2 1 0
                                                              1  1 channel
                                                            1 1  2 channels
                                                          1 1 1  3 channels
                                                        1 1 1 1  4 channels
                                                0 0 0 0            not used
                                          */
    char                 rh_name[12];    /* this file name */
    unsigned short       rh_chksum;      /* checksum for header */
    /* 128 bytes to here */
    char                 variable[896];  /* range gain, comments, and processing
                                            history */
} ;  /* 1024 bytes if tightly packed */
