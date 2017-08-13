
/* td0_lzss.c */
void init_decompress ( void );
void lzss_update ( int c );
unsigned short lzss_GetChar ( FILE *fp );
unsigned short GetBit ( FILE *fp );
unsigned short lzss_GetByte ( FILE *fp );
unsigned short lzss_DecodeChar ( FILE *fp );
unsigned short lzss_DecodePosition ( FILE *fp );
int lzss_getbyte ( FILE *fp );



