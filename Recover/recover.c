#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover image\n");
        return 1;
    }
    char *infile = argv[1];
    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }
    
    const size_t s = 512;
    size_t fileCount = 0;
    bool fileOpen = false;
    FILE *outFile = NULL;
    uint8_t buffer[s];
    while (!feof(inptr))
    {
        int count = fread(buffer, 1, s, inptr);
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && ((buffer[3] & 0xf0) == 0xe0))
        {
            if (fileOpen)
            {
                fclose(outFile);    
            }
            
            char fileName[8];
            sprintf(fileName, "%03zu.jpg", fileCount++);
            outFile = fopen(fileName, "w");
            fileOpen = true;
        }
        
        if (fileOpen)
            fwrite(buffer, count, 1, outFile);
    }
    if (fileOpen)
    {
        fclose(outFile);    
    }
    return 0;
}
