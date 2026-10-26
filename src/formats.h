#ifndef FORMATS_H
#define FORMATS_H
#include <cstdint>
#define MINHEADERSIZE 0x400

#ifndef _WIN32
#include <stdint.h>
typedef unsigned char BYTE;
typedef uint32_t      DWORD;
#else
#include <windows.h>
#endif
 


struct __attribute__((packed)) SsTraceHdrStruct
{ /* trace identification header */
    float trace_num;        /* trace sequence number within line */
    float position;         /* position along traverse */
    float num_samples;      /* samples per trace */
    float elevation;        /* elevation data if available */
    float unass5;           /* unassigned */
    float num_bytes;        /* bytes per sample; always 2 for Rev. 3 firmware */
    float aux_trace_num;    /* trace number again */
    float num_stacks;       /* number of stacks used to get trace */
    float time_window;      /* time window in ns */
    float unass10;          /* unassigned */
    float unass11;          /* unassigned */
    float unass12;          /* unassigned */
    float unass13;          /* unassigned */
    float unass14;          /* unassigned */
    float unass15;          /* unassigned */
    float unass16;          /* unassigned */
    float unass17;          /* unassigned */
    float unass18;          /* unassigned */
    float unass19;          /* unassigned */
    float unass20;          /* unassigned */
    float time_zero_adjust; /* where sample x = sample x + adjustment */
    float zero_flag;        /* 0 = data OK, 1 = zero data */
    float unass23;          /* unassigned */
    float time;             /* seconds past midnight */
    float comment_flag;     /* 1 = comment attached */
    char comment[28];       /* optional character comments */
};

// ----------------------------------------
// RADAN
// ----------------------------------------

#pragma pack(push, 1)
 
// structure member alignment macros
#define TYPEBYTE(x,n)   BYTE x[n]
#define SHORTBYTE(x)    TYPEBYTE(x,2)   // short int (16 bit)
#define FLOATBYTE(x)    TYPEBYTE(x,4)   // float
#define RFDATEBYTE(x)   TYPEBYTE(x,4)   // tagRFDate
#define COORDBYTE(x)    TYPEBYTE(x,8)   // tagRFCoords
 
struct tagRFDate                  // File header date/time structure
{
    unsigned sec2  : 5;           // second/2 (0-29)
    unsigned min   : 6;           // minute (0-59)
    unsigned hour  : 5;           // hour (0-23)
    unsigned day   : 5;           // day (1-31)
    unsigned month : 4;           // month (1=Jan, 2=Feb, etc.)
    unsigned year  : 7;           // year-1980 (0-127 = 1980-2107)
};
 
struct tagRFCoords                // Start/End position
{
    float rh_fstart;
    float rh_fend;
};
 
struct RGPS                       // GPS record/system time SYNC
{
    char   RecordType[4];         // "GGA"
    DWORD  TickCount;             // CPU tick count
    double PositionGPS[4];        // Latitude (positive if 'N'), Longitude (positive if 'E'),
                                  // Altitude, FIXUTC
};
 
// constants
const int MINHEADSIZE  = 1024;
const int PARAREASIZE  = 128;
const int GPSAREASIZE  = 2 * sizeof(RGPS);
const int INFOAREASIZE = MINHEADSIZE - PARAREASIZE - GPSAREASIZE;
 
struct tagRFHeader {                  /* Offset in bytes */
    short rh_tag;                     /* 00  0x00ff if header, 0xfnff for old file */
    short rh_data;                    /* 02  constant 1024 (obsolete) */
    short rh_nsamp;                   /* 04  samples per scan */
    short rh_bits;                    /* 06  bits per data word (8 or 16) */
    short rh_zero;                    /* 08  offset (0x80 or 0x8000 depends on rh_bits) */
    FLOATBYTE(rhf_sps);               /* 10  scans per second */
    FLOATBYTE(rhf_spm);               /* 14  scans per meter */
    FLOATBYTE(rhf_mpm);               /* 18  meters per mark */
    FLOATBYTE(rhf_position);          /* 22  position (ns) */
    FLOATBYTE(rhf_range);             /* 26  range (ns) */
    short rh_npass;                   /* 30  num of passes for 2-D files */
    RFDATEBYTE(rhb_cdt);              /* 32  creation date & time */
    RFDATEBYTE(rhb_mdt);              /* 36  last modification date & time */
    short rh_rgain;                   /* 40  offset to range gain function */
    short rh_nrgain;                  /* 42  size of range gain function */
    short rh_text;                    /* 44  offset to text */
    short rh_ntext;                   /* 46  size of text */
    short rh_proc;                    /* 48  offset to processing history */
    short rh_nproc;                   /* 50  size of processing history */
    short rh_nchan;                   /* 52  number of channels */
    FLOATBYTE(rhf_epsr);              /* 54  average dielectric constant */
    FLOATBYTE(rhf_top);               /* 58  position in meters */
    FLOATBYTE(rhf_depth);             /* 62  range in meters */
    COORDBYTE(rh_coordX);             /* 66  X coordinates */
    FLOATBYTE(rhf_servo_level);       /* 74  gain servo level */
    char reserved[3];                 /* 78  reserved */
    BYTE rh_accomp;                   /* 81  Ant Conf component */
    short rh_sconfig;                 /* 82  setup config number */
    short rh_spp;                     /* 84  scans per pass */
    short rh_linenum;                 /* 86  line number */
    COORDBYTE(rh_coordY);             /* 88  Y coordinates */
    BYTE rh_lineorder : 4;            /* 96 */
    BYTE rh_slicetype : 4;            /* 96 */
    char rh_dtype;                    /* 97 */
    char rh_antname[14];              /* 98  antenna name */
    BYTE rh_pass0TX : 4;              /* 112 active transmit mask */
    BYTE rh_pass1TX : 4;              /* 112 active transmit mask */
    BYTE rh_version : 3;              /* 113 1 - no GPS; 2 - GPS */
    BYTE rh_system : 5;               /* 113 3 for SIR3000 */
    char rh_name[12];                 /* 114 initial file name */
    short rh_chksum;                  /* 126 checksum for header */
    char variable[INFOAREASIZE];      /* 128 variable data */
    RGPS rh_RGPS[2];                  /* 944 GPS info */
};
 
#pragma pack(pop)
 
#endif /* RFHEADER_H */
