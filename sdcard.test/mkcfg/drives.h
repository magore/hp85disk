//@brief Drive index, Address, PPR and file name for emulated drive
typedef struct 
{
    uint8_t ADDRESS;    //< GPIB Address
    uint8_t PPR;        //< Parallel Poll Response Bit
    char     NAME[MAX_FILE_NAME_LEN+1]; // Filename of emulated image
} HeaderType;

//@brief Identify Bytes for Drives
typedef struct 
{
    uint16_t ID;        //<  Identify, For 9122 I1=02, I2=22H
} ConfigType;

// =============================================
///@brief Printer structure 
typedef struct 
{
    HeaderType HEADER;
} PRINTERDeviceType;

// =============================================
typedef struct
{
    int16_t BYTES_PER_SECTOR;
    int16_t SECTORS_PER_TRACK;
    int16_t HEADS;
    int16_t CYLINDERS;
} AMIGOGemometryType;

///@brief AMIGO Disk structure - ID bytes and layout.
typedef struct 
{
    HeaderType HEADER;
    ConfigType CONFIG;
    AMIGOGemometryType GEOMETRY;
} AMIGODiskType;


// =============================================
///@brief SS80 Controller 
typedef struct {
    /*
        When packed this is 5 bytes
        CONTROLLER DESCRIPTION
        C1-C2    C1 = MSB, C2 = LSB
                 Installed unit byte; 1 bit for each unit.
                 Unit 15 is always present
        C3-C4    GPIB data transfer rate in kB/s on the bus
        C5       Controller Type
                 0 = CS/80 integrated single unit controller.
                 1 = CS/SO integrated multi-unit controller.
                 2 = CS/SO integrated multi-port controller.
                 4 = SS/SO integrated single unit controller.
                 5 = SS/80 integrated multi-unit controller.
                 6 SS/80 integrated multi-port controller.


    */
    uint16_t UNITS_INSTALLED;
    uint16_t TRANSFER_RATE;
    uint8_t TYPE;
} SS80ControllerType;

///@brief SS80 Unit 
typedef struct   //<  Unit description, 19 bytes
{
    /*
        When packed this is 19 bytes
        UNIT DESCRIPTION
        U1      Generic Unit Type, 0 = fixed, 1 - floppy, 2 = tape
                OR with 128 implies dumb can not detect media change
        U2-U4   Device number in BCD
                XX XX XY, X = BCD Unit number, Y = option number
        U5-U6   Number of bytes per block, MSB-LSB
        U7      Number of blocks which can be buffered
        U8      0 for SS80, Recommended burst size
        U9-U10  Block time in microseconds
        U11-U12 Continuous average transfer rate for long transfers kB/s
        U13-U14 Optimal retry time in 1O's of milliseconds
        U15-U16 Access time parameter in 1O's of milliseconds
        U17     Maximum Interleave factor
        U18     Fixed volume byte; one bit per volume (set if fixed);
        U19     Removable volume byte; one bit per volume (set if removable);
    */
    uint8_t UNIT_TYPE;
    uint32_t DEVICE_NUMBER;
    uint16_t BYTES_PER_BLOCK;
    uint8_t BUFFERED_BLOCKS;
    uint8_t BURST_SIZE;
    uint16_t BLOCK_TIME;
    uint16_t CONTINOUS_TRANSFER_RATE;
    uint16_t OPTIMAL_RETRY_TIME;
    uint16_t ACCESS_TIME;
    uint8_t MAXIMUM_INTERLEAVE;
    uint8_t FIXED_VOLUMES;
    uint8_t REMOVABLE_VOLUMES;
} SS80UnitType;

///@brief Volume Information Structure
typedef struct 
{
    /*
        When packed this is 13 bytes
        VOLUME DESCRIPTION
        SS80 units use single vecor mode so MAX CYLINDER,HEAD and SECTOR are not used
        V1-V3  Maximum value of cylinder address vector.
        V4     Maximum value of the head address vector.
        V5-V6  Maximum value of sector address vector.
               For devices that use bot the following expression must be true
               (V1,V2,V3+1)(V4+1)(V5,V6+1) = V7,V8,V9,V10,V11,V12+1
        V7-V12 Maximum value of single vector address in blocks.
        V13    Current Interleave Factor
    */
    uint32_t MAX_CYLINDER;
    uint8_t MAX_HEAD;
    uint16_t MAX_SECTOR;
    uint32_t MAX_BLOCK_NUMBER;
    uint8_t INTERLEAVE;
} SS80VolumeType;


///@brief Disk Information Structure
typedef struct 
{
    HeaderType HEADER;
    ConfigType CONFIG;
    SS80ControllerType CONTROLLER;
    SS80UnitType UNIT;
    SS80VolumeType VOLUME;
} SS80DiskType;
// =============================================

///@brief SS80 Disk Definitions
SS80DiskType SS80DiskDefault =
{
    {
        0,          // GPIB Address
        0,          // PPR
        "" 			// FILE name
    },
    {
        0,      	// ID
    },
    {
        0x8001,     // Installed Units 1 (15 is always on)
        744,        // GPIB data transfer rate in kB/s on the bus
        4,          // 5 = SS/80 integrated multi-unit controller
    },
    {
        0,          // Generic Unit Type, 0 = fixed
        0,    		// BCD Device number XX XX XY, X=Unit, Y=option
        0,          // Bytes per block
        1,          // Buffered Blocks
        0,          // Burst size = 0 for SS80
        0x1F6,      // Block time in microseconds
        140,        // Continous transfer time in kB/s
        4500,       // Retry time in 0.01 secods
        4500,       // Access time in 0.01 seconds
        31,         // Maximum interleave factor
        1,          // Fixed volume byte, one bit per volume
        0           // Removable volume byte, one bit per volume
    },
    {
        0,          // Maximum Cylinder  - not used
        0,          // Maximum Head      - not used
        0,          // Maximum Sector    - not used
        0,          // Maximum Block Number
        2           // Interleave
    }
};

AMIGODiskType AMIGODiskDefault =
{
    {
        0,          // GPIB Address
        0,          // PPR
        ""    		// FILE name
    },
    {
        0     ,     // ID
    },
    {
        0,          // Bytes Per Sector
        0,          // Sectors Per Track
        0,          // Sides
        0			// Cylinders
    }
};
