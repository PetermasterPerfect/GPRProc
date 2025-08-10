#include <cstdint>
#define MINHEADERSIZE 0x400

typedef uint8_t  BYTE;
typedef uint32_t DWORD;


struct SsTraceHdrStruct
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
// A. Internal structures
// ----------------------------------------

struct __attribute__((packed)) tagRFDate // File header date/time structure
{
    unsigned sec2  : 5; // second/2 (0-29)
    unsigned min   : 6; // minute (0-59)
    unsigned hour  : 5; // hour (0-23)
    unsigned day   : 5; // day (1-31)
    unsigned month : 4; // month (1=Jan, 2=Feb, etc.)
    unsigned year  : 7; // year-1980 (0-127 = 1980-2107)
};

struct __attribute__((packed)) tagRFCoords // Start/End position
{
    float rh_fstart;
    float rh_fend;
};

struct __attribute__((packed)) RGPS // GPS record/system time SYNC
{
    char  RecordType[4];  // "GGA"
    DWORD TickCount;      // CPU tick count
    double PositionGPS[4]; // Latitude, Longitude, Altitude, FIXUTC
};

// ----------------------------------------
// C. Radan Header structure
// ----------------------------------------

struct __attribute__((packed)) tagRFHeader
{
    // Offset in bytes indicated in comments
    int16_t rh_tag;      // 0x00ff if header, 0xfnff for old file 00
    int16_t rh_data;     // Offset to Data from beginning of file 02
    int16_t rh_nsamp;    // samples per scan 04
    int16_t rh_bits;     // bits per data word (8,16, 32) 06
    int16_t rh_zero;     // repeats/sample or special flags 08

    float rhf_sps;       // scans per second 10
    float rhf_spm;       // scans per meter 14
    float rhf_mpm;       // meters per mark 18
    float rhf_position;  // position (ns) 22
    float rhf_range;     // range (ns) 26

    int16_t rh_npass;    // num of passes for 2-D files 30
    struct tagRFDate rhb_cdt; // Creation date & time 32
    struct tagRFDate rhb_mdt; // Last modification date & time 36

    int16_t rh_mapOffset; // For internal use 40
    int16_t rh_mapSize;   // For internal use 42
    int16_t rh_text;      // offset to text 44
    int16_t rh_ntext;     // size of text 46
    int16_t rh_proc;      // offset to processing history 48
    int16_t rh_nproc;     // size of processing history 50
    int16_t rh_nchan;     // number of channels 52

    float rhf_epsr;       // average dielectric constant 54
    float rhf_top;        // position in meters 58
    float rhf_depth;      // range in meters 62
    struct tagRFCoords rh_coordX; // X coordinates 66

    float rhf_servo_level; // gain servo level 74
    char  reserved[3];     // reserved 78
    BYTE  rh_accomp;       // Ant Conf component 81

    int16_t rh_sconfig;   // setup config number 82
    int16_t rh_spp;       // scans per pass 84
    int16_t rh_linenum;   // line number 86
    struct tagRFCoords rh_coordY; // Y coordinates 88

    BYTE  rh_lineorder : 4; // 96
    BYTE  rh_slicetype : 4; // 96

    char  rh_dtype;        // 97
    char  rh_antname[14];  // Antenna name 98

    BYTE  rh_pass0TX  : 4; // Active Transmit mask 112
    BYTE  rh_pass1TX  : 4; // Active Transmit mask 112
    BYTE  rh_version  : 3; // 1 – no GPS; 2 - GPS 113
    BYTE  rh_system   : 5; // system type 113

    char  rh_name[12];     // Initial File Name 114
    int16_t rh_chksum;     // checksum for header 126

    char  variable[768];   // Variable data 128 (INFOAREASIZE resolved)
    struct RGPS rh_RGPS[2];// GPS info 944
};

