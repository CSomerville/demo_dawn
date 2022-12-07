#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read: %s\n", path);
        exit(74);
    }
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file: %s\n", path);
        exit(74);
    }
    
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

/* shamelessly ripped from https://stackoverflow.com/questions/55766058/how-can-i-generate-random-doubles-in-c
 */
double rand_double(void) {
	uint64_t r53 = ((uint64_t)(rand()) << 21) ^ (rand() >> 2);
	return (double)r53 / 9007199254740991.0; // 2^53 - 1
}
