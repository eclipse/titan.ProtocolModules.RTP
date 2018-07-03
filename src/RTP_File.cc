/******************************************************************************
* Copyright (c) 2005, 2014  Ericsson AB
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v2.0
* which accompanies this distribution, and is available at
* https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
*
* Contributors:
*   Gabor Szalai - initial implementation and initial documentation
*   Peter Kremer
*   Sandor Palugyai
*   Tibor Harai
******************************************************************************/
//
//  File:               RTP_File.cc
//  Description:        Media files operation for RTP
//  Rev:                R5A
//  Prodnr:             CNL 113 392
//

#include "RTP_File_Types.hh"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define CREAT_MODE  0644
namespace RTP__File__Types{
enum Operation {READ, WRITE};

typedef struct{
    CHARSTRING     *filename;
    int     fd;
    int     block_size;
    struct stat stat_buf;
} File_info_type;

void log_info_list();
int f_Fileinfo_Check(const int& fd, const Operation& OPERATION);
int f_Operation_Check(const int& fd, const int& blockno, const int& nof_b, const Operation& OPERATION, const int& hdr_off);
void f__Count__JPEG__Header__Offset(RTP__FileInfo& fi);

File_info_type *file_info_list = NULL;
int nof_file_infos = 0;

INTEGER f__INIT__CODEC(const CHARSTRING& filename,
                       const INTEGER& block_size,
                       const InitOperType& INIT_TYPE)
{
    if(filename=="")
        TTCN_error("INIT__CODEC: empty filename is not allowed");
    if((int)block_size<=0)
        TTCN_error("INIT__CODEC: Block size must be a positive integer");
    if( !(file_info_list = (File_info_type*) realloc( file_info_list, (nof_file_infos+1)*sizeof(File_info_type) )) )
        TTCN_error("INIT__CODEC: There is not enough memory.");
    switch (INIT_TYPE) {
        case InitOperType::OPEN:
            if( (file_info_list[nof_file_infos].fd = open((const char *)filename, O_RDONLY))<0 )
                TTCN_error("INIT__CODEC: Cannot open file '%s'\n", (const char *)filename);
            break;
        case InitOperType::CREATE:
            if( (file_info_list[nof_file_infos].fd = creat((const char *)filename, CREAT_MODE))<0 )
                TTCN_error("INIT__CODEC: Cannot create file '%s'\n", (const char *)filename);
            break;
        default:
            TTCN_error("INIT__CODEC: Wrong init_type setting! Available: OPEN, CREATE.\n");
            break;
    }
    file_info_list[nof_file_infos].filename = new CHARSTRING(filename);
    file_info_list[nof_file_infos].block_size = (int)block_size;
    if( stat((const char *)*file_info_list[nof_file_infos].filename, &file_info_list[nof_file_infos].stat_buf) == -1 )
        TTCN_error("INIT__CODEC: Cannot gather file info");
    if( file_info_list[nof_file_infos].stat_buf.st_size%block_size != 0 )
        TTCN_warning("INIT__CODEC: File %s contains uncomplete blocks", (const char *)filename);

    TTCN_Logger::log(TTCN_DEBUG,"INIT__CODEC: filename: %s, origfilename: %s\n", (const char *)*file_info_list[nof_file_infos].filename, (const char*)filename);

    nof_file_infos++;
    log_info_list();
    return INTEGER(file_info_list[nof_file_infos-1].fd);
}

OCTETSTRING f__GET__CONTENT(const INTEGER& fd,
                            const INTEGER& blockno,
                            const INTEGER& nof_blocks_to_read,
                            const INTEGER& header_offset)
{
    int bytes_to_read = f_Operation_Check((int)fd, (int)blockno, (int)nof_blocks_to_read, READ, (int)header_offset);
    
    if (!bytes_to_read)
        return OCTETSTRING(0, NULL);

    unsigned char * buf = (unsigned char*) malloc(bytes_to_read);
    if(!buf)
        TTCN_error("GET_CONTENT: There is not enough memory.");

    int read_data = 0;
    if((read_data = read(fd, buf, bytes_to_read))<0)
        TTCN_error("GET_CONTENT: unsuccesful read (%d)", read_data);
    OCTETSTRING retval(read_data, buf);
    free(buf);
    return retval;
}


void f__Count__JPEG__Header__Offset(RTP__FileInfo& fi)
{
    int i = f_Fileinfo_Check((int)fi.fd(), READ);
    struct stat &stat_buf = file_info_list[i].stat_buf;
    int hdr_off = (int)fi.headerOffset();

    if(hdr_off < 0)
        TTCN_error("Get_Media_Header: Header offset must be a non-negative integer!");
    if(stat_buf.st_size < hdr_off)
        TTCN_error("Get_Media_Header: Size of file %s is smaller than the size of header!", (const char *)*file_info_list[i].filename);

    if(lseek((int)fi.fd(), 0, SEEK_SET) < 0)
        TTCN_error("Count_JPEG_Header_Offset: Cannot set the starting position in file %s\n", (const char *)*file_info_list[i].filename);

    unsigned char * buf = (unsigned char*) malloc(hdr_off);
    if(!buf)
        TTCN_error("Count_JPEG_Header_Offset: There is not enough memory.");
    int read_data = 0;
    if((read_data = read((int)fi.fd(), buf, hdr_off))<0)
        TTCN_error("Count_JPEG_Header_Offset: unsuccesful read (%d)",read_data);

    fi.headerOffset() = hdr_off + ((((int)buf[hdr_off-2]) << 8) | buf[hdr_off-1]);
    free(buf);
}

INTEGER f__PUT__CONTENT(const INTEGER& fd,
                        const INTEGER& blockno,
                        const OCTETSTRING& stream,
                        const INTEGER& header_offset)
{
    int bytes_to_write = f_Operation_Check((int)fd, (int)blockno, stream.lengthof(), WRITE, (int)header_offset);
    
    if (!bytes_to_write)
        return 0;

    unsigned char * buf = (unsigned char*)(const unsigned char*) stream;
    if (write(fd, buf, bytes_to_write) != bytes_to_write)
        TTCN_error("PUT_CONTENT: unsuccesful write to file");

    return bytes_to_write;
}

void f__CLOSE__CODEC(const INTEGER& fd)
{
    for(int i=0;i<nof_file_infos;i++) {
        if(file_info_list[i].fd == fd) {
            close((int)fd);
            delete file_info_list[i].filename;
            nof_file_infos--;
            if(nof_file_infos==0) {
                free(file_info_list);
                file_info_list = NULL;
                return;
            }
            for(int j=i;j<nof_file_infos;j++)
                file_info_list[j] = file_info_list[j+1];
            if( !(file_info_list = (File_info_type *) realloc(file_info_list, nof_file_infos*sizeof(File_info_type))) )
                TTCN_error("CLOSE__CODEC: There is not enough memory.");
            return;
        }
    }
    TTCN_error("CLOSE__CODEC: Unknown file descriptor (%d)", (int)fd);
}

void log_info_list()
{
    TTCN_Logger::begin_event(TTCN_DEBUG);
    TTCN_Logger::log_event("\nnumber of fds: %d\n", nof_file_infos);
    for(int i=0;i<nof_file_infos;i++) {
        TTCN_Logger::log_event("%d. fd = %d\n", i, file_info_list[i].fd);
    }
    TTCN_Logger::end_event();
}

int f_Fileinfo_Check(const int& fd, const Operation& OPERATION)
{
    int i;
    for(i=0; i<nof_file_infos; i++) {
        if(file_info_list[i].fd == fd) break;
    }
    if(i == nof_file_infos)
        TTCN_error("Fileinfo_Check: Unknown file descriptor (%d)", fd);
    if(!nof_file_infos)
        TTCN_error("Fileinfo_Check: file info list is empty");
    if(!file_info_list[i].filename)
        TTCN_error("Fileinfo_Check: inconsistent file info list (filename is missing)");
    if( (file_info_list && !nof_file_infos) || (!file_info_list && nof_file_infos) )
        TTCN_error("Fileinfo_Check: inconsistent file info list");
    if (OPERATION==WRITE && stat((const char *)*file_info_list[i].filename, &file_info_list[i].stat_buf)==-1)
        TTCN_error("Fileinfo_Check: Cannot gather file info");
    return i;
}

int f_Operation_Check(const int& fd, 
                      const int& blockno,
                      const int& nof_b,
                      const Operation& OPERATION,
                      const int& hdr_off)
{
    int i = f_Fileinfo_Check(fd, OPERATION);
    int block_size = file_info_list[i].block_size;
    struct stat &stat_buf = file_info_list[i].stat_buf;

    // nof_b: number of blocks (OPERATION==READ) or
    //        number of bytes (OPERATION==WRITE)
    if(!nof_b && !hdr_off)
        return 0;
    if(nof_b < 0)
        TTCN_error("Operation_Check: The number of blocks to read must be a non-negative integer");
    if(blockno < 0)
        TTCN_error("Operation_Check: Starting block position must be a non-negative integer");
    if(hdr_off < 0)
        TTCN_error("Operation_Check: Header offset must be a non-negative integer");
    if(stat_buf.st_size < blockno*block_size + hdr_off)
        TTCN_error("Operation_Check: Size of file %s is smaller than the starting block position", (const char *)*file_info_list[i].filename);

    int bytes_to_operation;
    switch (OPERATION) {
        case READ:
            if(!nof_b && hdr_off) {
                if(lseek(fd, 0, SEEK_SET) < 0)
                    TTCN_error("Operation_Check: Cannot set the starting position in file %s\n", (const char *)*file_info_list[i].filename);
                return hdr_off;
            }
            if(lseek(fd, blockno*block_size + hdr_off, SEEK_SET) < 0)
                TTCN_error("Operation_Check: Cannot set the starting position in file %s\n", (const char *)*file_info_list[i].filename);
            if (stat_buf.st_size == blockno*block_size + hdr_off)
                bytes_to_operation = 0;
            else if (stat_buf.st_size < (blockno+nof_b)*block_size + hdr_off) {
                bytes_to_operation = stat_buf.st_size - block_size*blockno - hdr_off;
                TTCN_warning("Operation_Check: This is an uncomplete block. Size: %d byte/bytes.", bytes_to_operation);
            }
            else
                bytes_to_operation = block_size*nof_b;
            break;
        case WRITE:
            if(lseek(fd, blockno*block_size + hdr_off, SEEK_SET) < 0)
                TTCN_error("Operation_Check: Cannot set the starting position in file %s\n", (const char *)*file_info_list[i].filename);
            bytes_to_operation = nof_b;
            break;
        default:
            TTCN_error("Operation_Check: Wrong OPERATION setting! Available: READ, WRITE.\n");
            break;
    }
    return bytes_to_operation;
}

}
