/*
// Stripped down version to just extract RAW LIF images
// Modifications by Mike Gore July 2017
//
// Copyright (C) 2006-2014 Jean-François DEL NERO
//
// This file is part of the HxCFloppyEmulator library
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : teledisk_loader.c
// Contains: Teledisk (TD0) floppy image loader
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

///@brief  Teledisk loader is a part of the HxCFloppyEmulator project
/// Modified for stand alone operation to extract RAW LIF images
/// by Mike Gore July 2017 
/// Source: https://github.com/jfdelnero/libhxcfe/tree/master/sources/loaders/teledisk_loader

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <inttypes.h>
#include "crc.h"

///@brief teledisk to lif header
#include "td02lif.h"

#include "teledisk_loader.h"
#include "teledisk_format.h"
#include "td0_lzss.h"


int TeleDisk_libIsValidDiskFile(char * imgfile)
{
	int i;
	FILE * f;
	unsigned char crctable[32];
	unsigned char CRC16_High,CRC16_Low;
	TELEDISK_HEADER td_header;
	unsigned char * ptr;

    if(!imgfile || !strlen(imgfile))
    {
        printf("Expected TD0 filename\n");
        return(0);
    }


    f=fopen(imgfile,"rb");
    if(f==NULL)
    {
        printf("Can't open %s\n", imgfile);
        return(0);
    }

    fseek(f,0,SEEK_SET);
    memset(&td_header,0,sizeof(TELEDISK_HEADER));

    if( fread( &td_header, sizeof(TELEDISK_HEADER), 1, f ) != sizeof(TELEDISK_HEADER) )
    {
        printf("Teledisk bas header\n");
        return(0);
    }
    if ( ((td_header.TXT[0]!='t') || (td_header.TXT[1]!='d')) && ((td_header.TXT[0]!='T') || (td_header.TXT[1]!='D')))
    {
        printf("TeleDisk header tag error\n");
        fclose(f);
        return (0);
    }

    CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
    ptr=(unsigned char*)&td_header;
    for(i=0;i<0xA;i++)
    {
        CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
    }

    if(((td_header.CRC[1]<<8)|td_header.CRC[0]) != ((CRC16_High<<8)|CRC16_Low))
    {
        printf("Teledisk bad header crc\n");
        fclose(f);
        return (0);
    }

    printf("TeleDisk file\n");
    fclose(f);
    return (1);
}


int RLEExpander(unsigned char *src,unsigned char *dst,int blocklen)
{
  unsigned char *s1,*s2,d1,d2;
  unsigned char type;
  unsigned short len,len2,rlen;

  unsigned int uBlock;
  unsigned int uCount;

  s2=dst;

  len=(src[1]<<8)|src[0];
  type=src[2];
  len--;
  src=src+3;

  switch ( type )
  {
    case 0:
    {
	  memcpy(dst,src,len);
      rlen=len;
      break;
    }

	case 1:
    {
      len=(src[1]<<8) | src[0];
      rlen=len<<1;
      d1=src[2];
	  d2=src[3];
      while (len--)
	  {
		  *dst++=d1;
		  *dst++=d2;
	  }
	  src=src+4;
      break;
    }

	case 2:
	{
		rlen=0;
		len2=len;
		s1=&src[0];
		s2=dst;
		do
		{
			if( !s1[0])
			{
				len2--;

				len=s1[1];
				s1=s1+2;
				len2--;

				memcpy(s2,s1,len);
				rlen=rlen+len;
				s2=s2+len;
				s1=s1+len;

				len2=len2-len;
			}
			else
			{
				uBlock = 1<<*s1;
				s1++;
				len2--;

				uCount = *s1;
				s1++;
				len2--;

				while(uCount)
				{
						memcpy(s2, s1, uBlock);
						rlen=rlen+uBlock;
						s2=s2+uBlock;
						uCount--;
				}

				s1=s1 + uBlock;
				len2=len2-uBlock;
			}

		}while(len2);
    }

    default:
    {
      rlen=-1;
    }
  }

  return rlen;
}

int TeleDisk_libLoad_DiskFile(floppy_t *floppydisk, char * imgfile, void *data)
{
	FILE * f;
	unsigned int i;
	unsigned int file_offset;
	unsigned char skew,trackformat;
	unsigned short sectorsize;
	int Compress,numberoftrack,sidenumber;
	unsigned short * datalen;
	TELEDISK_HEADER        *td_header;
	TELEDISK_TRACK_HEADER  *td_track_header;
	TELEDISK_SECTOR_HEADER *td_sector_header;
	TELEDISK_COMMENT * td_comment;
	unsigned char tempdata[8*1024];
	unsigned char crctable[32];
	unsigned char CRC16_High,CRC16_Low;
	unsigned char * ptr;
	long size, filesize;
	unsigned char * fileimage;
	uint32_t fileimage_buffer_offset;
	int rlen;
    int fix = 0;
    int status = 1;

	sector_config_t * sectorconfig;

	printf("TeleDisk image file %s\n",imgfile);

    if(!imgfile || !strlen(imgfile))
    {
        printf("Expected TD0 filename\n");
        return(0);
    }


    f=fopen(imgfile,"rb");
    if(f==NULL)
    {
        printf("TeleDisk Can't open: %s\n",imgfile);
		return (0);
	}

	fseek(f,0,SEEK_END);
	filesize=ftell(f);
    printf("image:[%s], size:[%ld]\n", imgfile,filesize);

	if(!filesize)
	{
		printf("TeleDisk 0 byte file\n");
		fclose(f);
		return (0);
	}
	fseek(f,0,SEEK_SET);

	fileimage_buffer_offset=0;
	fileimage=(unsigned char*)calloc(filesize+512,1);
	if(!fileimage)
	{
		printf("TeleDisk Calloc error !\n");
		fclose(f);
		return (0);
	}

	memset(fileimage,0,filesize+512);
	size = fread(fileimage,1,filesize,f);
	if(size != filesize)
    {
        printf("%s read expected %ld returned %ld\n", imgfile, filesize,size);
    }
    fclose(f);


	td_header=(TELEDISK_HEADER*)&fileimage[fileimage_buffer_offset];
	fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_HEADER);

	if ( ((td_header->TXT[0]!='t') || (td_header->TXT[1]!='d')) && ((td_header->TXT[0]!='T') || (td_header->TXT[1]!='D')))
	{
		printf("TeleDisk bad header tag:[%c%c]\n", (int)td_header->TXT[0],(int)td_header->TXT[1]);
		free(fileimage);
		return (0);
	}

	CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
	ptr=(unsigned char*)td_header;
	for(i=0;i<0xA;i++)
	{
		CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
	}

	if(((td_header->CRC[1]<<8)|td_header->CRC[0])!=((CRC16_High<<8)|CRC16_Low))
	{
		printf("TeleDisk bad header crc\n");
		free(fileimage);
		return (0);
	}

	printf("TeleDisk version: %d\n",td_header->TDVer);
	if((td_header->TDVer>21) || (td_header->TDVer<10))
	{
		printf("TeleDisk Unsupported version:%d\n", td_header->TDVer);
		free(fileimage);
		return (0);
	}

	Compress=0;
	if(((td_header->TXT[0]=='T') && (td_header->TXT[1]=='D')))
	{
		printf("TeleDisk Normal compression\n");
		Compress=0;
	}

	if(((td_header->TXT[0]=='t') && (td_header->TXT[1]=='d')))
	{
		printf("TeleDisk Advanced compression\n");
		fileimage=unpack(fileimage,filesize);
		Compress=1;
	}

	td_header=(TELEDISK_HEADER*)&fileimage[0];

	if(td_header->TrkDens&0x80)
	{

		CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);

		td_comment=(TELEDISK_COMMENT *)&fileimage[fileimage_buffer_offset];
		fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_COMMENT);

		//fread( &td_comment, sizeof(td_comment), 1, f );
		ptr=(unsigned char*)td_comment;
		ptr=ptr+2;
		for(i=0;i<sizeof(TELEDISK_COMMENT)-2;i++)
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );

		memcpy(&tempdata,&fileimage[fileimage_buffer_offset],td_comment->Len);
		fileimage_buffer_offset=fileimage_buffer_offset+td_comment->Len;

		ptr=(unsigned char*)&tempdata;
		for(i=0;i<td_comment->Len;i++)
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );


		printf("TeleDisk date: %.2d/%.2d/%.4d %.2d:%.2d:%.2d\n",
            (int)td_comment->bDay,
            (int)td_comment->bMon+1,
            (int)td_comment->bYear+1900,
            (int)td_comment->bHour,
            (int)td_comment->bMin,
            (int)td_comment->bSec);
        printf("TeleDisk Comment:[%s]\n",tempdata);

	}

	numberoftrack=0;
	sectorsize=512;

	file_offset=fileimage_buffer_offset;

	floppydisk->floppyNumberOfSide=td_header->Surface;

	td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
	fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);

	while(td_track_header->SecPerTrk!=0xFF)
	{
		if(td_track_header->PhysCyl>numberoftrack)
		{
			numberoftrack=td_track_header->PhysCyl;
		}
		CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
		ptr=(unsigned char*)td_track_header;
		for(i=0;i<0xA;i++)
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );

		for ( i=0;i < td_track_header->SecPerTrk;i++ )
		{
			td_sector_header=(TELEDISK_SECTOR_HEADER  *)&fileimage[fileimage_buffer_offset];
			fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_SECTOR_HEADER);

			if  ( (td_sector_header->Syndrome & 0x30) == 0 && (td_sector_header->SLen & 0xf8) == 0 )
			{
				//fileimage_buffer_offset=fileimage_buffer_offset+sizeof(unsigned short);

				datalen=(unsigned short*)&fileimage[fileimage_buffer_offset];
				fileimage_buffer_offset=fileimage_buffer_offset+(*datalen)+2;
			}
		}
		td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
		fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);
	}

	floppydisk->floppyNumberOfTrack=numberoftrack+1;
	floppydisk->floppySectorPerTrack=-1;

	//Source disk density (0 = 250K bps,  1 = 300K bps,  2 = 500K bps ; +128 = single-density FM)
	switch(td_header->Dens)
	{
		case 0:
			floppydisk->floppyBitRate=250000;
			break;
		case 1:
			floppydisk->floppyBitRate=300000;
			break;
		case 2:
			floppydisk->floppyBitRate=500000;
			break;
		default:
			floppydisk->floppyBitRate=250000;
			break;
	}


	skew=1;

	//////////////////////////////////
	fileimage_buffer_offset=file_offset;

	td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
	fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);

	printf("%d tracks, %d side(s), sectors(%d),bitrate:%ld\n",
        (int)floppydisk->floppyNumberOfTrack,
        (int)floppydisk->floppyNumberOfSide,
        (int)td_track_header->SecPerTrk,
        (long)floppydisk->floppyBitRate);

	while(td_track_header->SecPerTrk!=0xFF)
	{
		if(td_track_header->PhysSide&0x7F)
			sidenumber=1;
		else
			sidenumber=0;

		if(td_track_header->PhysSide&0x80)
			trackformat=IBMFORMAT_SD;
		else
			trackformat=IBMFORMAT_DD;

        if(debuglevel & 0x400)
            printf("------ Track:%d, Side:%d, Number of Sectors:%02d -----\n",
                td_track_header->PhysCyl,
                sidenumber,td_track_header->SecPerTrk);

		////////////////////crc track header///////////////////
		CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
		ptr=(unsigned char*)td_track_header;
		for(i=0;i<0x3;i++)
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
		if(CRC16_Low!=td_track_header->CRC)
			printf("!!!! Track header CRC Error !!!!\n");
		////////////////////////////////////////////////////////

		sectorconfig=(sector_config_t *)calloc(sizeof(sector_config_t)*td_track_header->SecPerTrk,1);
		memset(sectorconfig,0,sizeof(sector_config_t)*td_track_header->SecPerTrk);


		for ( i=0;i < td_track_header->SecPerTrk;i++ )
		{
			td_sector_header=(TELEDISK_SECTOR_HEADER  *)&fileimage[fileimage_buffer_offset];
			fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_SECTOR_HEADER);

			sectorconfig[i].cylinder=td_sector_header->Cyl;
			sectorconfig[i].head=td_sector_header->Side;
			sectorconfig[i].sector=td_sector_header->SNum;
			sectorconfig[i].sectorsize=128<<td_sector_header->SLen;
			sectorconfig[i].bitrate=floppydisk->floppyBitRate;
			sectorconfig[i].trackencoding=trackformat;

			if(td_sector_header->Syndrome & 0x04)
			{
				sectorconfig[i].use_alternate_datamark=1;
				sectorconfig[i].alternate_datamark=0xF8;
			}

			if(td_sector_header->Syndrome & 0x02)
			{
				sectorconfig[i].use_alternate_data_crc=2;
			}

			if(td_sector_header->Syndrome & 0x20)
			{
				sectorconfig[i].missingdataaddressmark=1;
			}

			sectorconfig[i].input_data=calloc(sectorconfig[i].sectorsize,1);
			if  ( (td_sector_header->Syndrome & 0x30) == 0 && (td_sector_header->SLen & 0xf8) == 0 )
			{
				datalen=(unsigned short*)&fileimage[fileimage_buffer_offset];
				memcpy(&tempdata,&fileimage[fileimage_buffer_offset],(*datalen)+2);
				fileimage_buffer_offset=fileimage_buffer_offset+(*datalen)+2;
				rlen=RLEExpander(tempdata,sectorconfig[i].input_data,(int)*datalen);
			}
			else
			{
				memset(sectorconfig[i].input_data,0,sectorconfig[i].sectorsize);
			}

        }

///======================================================================
///@brief LIF extraction code hook
///       by Mike Gore 2017
///
        status = td02lif_track(sectorconfig, td_track_header->SecPerTrk, data);
///
///======================================================================

		for ( i=0;i < td_track_header->SecPerTrk;i++ )
		{
			if(sectorconfig[i].input_data)
				free(sectorconfig[i].input_data);
		}
		free(sectorconfig);

		td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
		fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);
        ///@brief Teledisk to LIF error exit
        if(!status)
            break;

	}  // while(td_track_header->SecPerTrk!=0xFF)

	free(fileimage);
	return (status);
}
