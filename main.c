//
//   bin2sphere
//
//   Utility to wrap raw program data in a Sphere-compatible block suitable for
//   storage and retrieval through the tape cassette mechanisms.
//
//   The Sphere cassette format uses the 300bps Kansas City/Byte standard
//   for raw byte stream audio. The logical data format consists of one or more
//   named "blocks" of data stored in a binary format. The format of a block is 
//   as follows:
//     - 3x sync bytes (0x16)
//     - One escape marker (0x1B)
//     - Two bytes data length (stored big endian)
//     - Two bytes block "name" (typically ASCII)
//     - Binary data bytes (count of bytes equal to length given above, *plus 
//        one*)
//     - End of transmission marker (0x17)
//     - Checksum byte (see below)
//     - Three additional trailer bytes (typically same value as checksum)
//   The checksum is computed as a running sum of the values in the data portion
//   (only). An 8-bit counter is used for the sum which just rolls over as 
//   needed.
//
//   A cassette can have more than one of these blocks of data present, which
//   are differentiated by the user by "name". Unlike some other tape formats,
//   there is no information about load address stored on the tape; the user is
//   expected to provide the load address (and request the block by name).
//
//   This utility will accept any binary file as input, along with a block name,
//   and emit the correct header and trailer surrounding the original data.
//   This utility does NOT produce audio cassette data. Its output can be 
//   fed into a utility that does, or the cassette block output can be loaded
//   under emulation, etc.
//
//  Copyright (c) Ben Zotto 2023.
//  See LICENSE for licensing information.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HEADER_SYNC     0x16
#define HEADER_ESC      0x1B
#define TRAILER_EOT     0x17

void print_usage(const char * name) {
    printf("usage: %s block_name input_file output_file\n", name);
}

int main(int argc, char **argv) {
    
    if (argc != 4) {
        print_usage(argv[0]);
        return -1;
    }

    const char * block_name = argv[1];
    const char * input_file_name = argv[2];
    const char * output_file_name = argv[3];
    
    // Check the block name.
    if (block_name == NULL || strlen(block_name) != 2) {
        printf("Block name must be two ASCII characters\n");
        return -1;
    }
        
    if (block_name[0] <= ' ' || block_name[0] > '_' ||
        block_name[1] <= ' ' || block_name[1] > '_') {
        printf("Warning: block name uses character(s) that are outside normal Sphere caps alphanumeric range.\n");
    }
        
    // Open the input file.
    FILE * file = fopen(input_file_name, "rb");
    if (!file) {
        printf("Unable to open %s\n", input_file_name);
        return -1;
    }
    
    if (fseek(file, 0 , SEEK_END) != 0) {
        printf("Error reading %s\n", input_file_name);
        return -1;
    }

    long file_size = ftell(file);

    if (file_size == 0) {
        printf("Input file is empty!");
        fclose(file);
        return -1;
    }

    if (file_size >= 0xFFFF) {
        printf("Input file too big! (Must be less than 64kbytes)");
        fclose(file);
        return -1;
    }

    long data_size = file_size + 13;

    unsigned char * data = malloc(data_size + 13);
    if (data == NULL) {
        printf("File too large to allocate work buffer");
        fclose(file);
        return -1;
    }
    
    if (fseek(file, 0 , SEEK_SET) != 0) {
        printf("Error reading %s\n", input_file_name);
        free(data);
        fclose(file);
        return -1;
    }

    unsigned char * ptr = data;
    *ptr++ = HEADER_SYNC;
    *ptr++ = HEADER_SYNC;
    *ptr++ = HEADER_SYNC;
    *ptr++ = HEADER_ESC;
    long header_size = file_size - 1;
    *ptr++ = (header_size >> 8) & 0xFF;
    *ptr++ = header_size & 0xFF;
    *ptr++ = block_name[0];
    *ptr++ = block_name[1];
    
    unsigned long bytes_read = fread(ptr, 1, file_size, file);
    fclose(file);
    if (bytes_read != file_size) {
        printf("Error reading %s\n", input_file_name);
        free(data);
        return -1;
    }
    
    unsigned char checksum = 0;
    for (long i = 0; i < bytes_read; i++) {
        checksum += ptr[i];
    }
    ptr += bytes_read;
    
    *ptr++ = TRAILER_EOT;
    *ptr++ = checksum;
    *ptr++ = checksum;
    *ptr++ = checksum;
    *ptr = checksum;
    
    file = fopen(output_file_name, "wb");
    if (!file) {
        printf("Unable to open %s\n", output_file_name);
        free(data);
        return -1;
    }
    
    unsigned long bytes_written = fwrite(data, 1, data_size, file);
    fclose(file);
    if (bytes_written != data_size) {
        printf("Error writing %s\n", output_file_name);
        free(data);
        return -1;   
    }
    
    // Finished with no issues.
    free(data);
}
