#include "bmp.h"
#include "stdio.h"
#include "cs50.h"
#include "string.h"
#include "math.h"

#define SUCCESS 0
#define FAILURE 1
typedef struct
{
    uint8_t result;
    float n;
    char *in;
    char *out;
} UserInput;

UserInput* validateInput(int args, const char ** argv)
{
    UserInput *input = (UserInput*)malloc(sizeof(UserInput));
    input->result = FAILURE;
    input->in = NULL;
    input->out = NULL;
    if (args == 4)
    {
        float n = atof(argv[1]);
        if (n >= 0.0 && n <= 100)
        {
            input->result = SUCCESS;
            input->n = n;
            input->in = (char*)malloc(sizeof(char)*strlen(argv[2])+1);
            input->out = (char*)malloc(sizeof(char)*strlen(argv[3])+1);
            memcpy(input->in, argv[2], strlen(argv[2])+1);
            memcpy(input->out, argv[3], strlen(argv[3])+1);
        }
    }
    return input;
}

void freeUserInput(UserInput **pIn)
{
    if (*pIn != NULL)
    {
        if ((*pIn)->in != NULL)
            free((*pIn)->in);
        if ((*pIn)->out != NULL)
            free((*pIn)->out);
        free(*pIn);
    }
}

void resize(BITMAPINFOHEADER *ri, float n)
{
    ri->biWidth = round(ri->biWidth * n);
    ri->biHeight = round(ri->biHeight * n);
    int padding = (4 - (ri->biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    ri->biSizeImage = abs(ri->biHeight) * ((ri->biWidth * 3) + padding);
}


int main(int args, const char ** argv)
{
    UserInput *in = validateInput(args, argv);
    if (in->result == FAILURE)
    {
        printf("Usage: ./resize f infile outfile\n");
        freeUserInput(&in);
        return 1;
    }
    
    
    // remember filenames
    char *infile = in->in;
    char *outfile = in->out;

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    
    BITMAPINFOHEADER ri = bi;
    resize(&ri, in->n);
    
    // resize bf.bfSize
    int rgbSize = ri.biSizeImage + bi.biSize + 14;
    bf.bfSize = rgbSize;
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&ri, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int outPadding = (4 - (ri.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    RGBTRIPLE triple[abs(bi.biHeight)][bi.biWidth];

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // read RGB triple from infile
            fread(&triple[i][j], sizeof(RGBTRIPLE), 1, inptr);
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);
    }
    for (int h = 0, riHeight = abs(ri.biHeight), n = round(in->n); h < riHeight; ++h)
    {
        for (int w = 0; w < ri.biWidth; ++w)
        {
            int i = (in->n >= 1) ? h/in->n : roundf(h/in->n);
            int j = (in->n >= 1) ? w/in->n : roundf(w/in->n);
            fwrite(&triple[i][j], sizeof(RGBTRIPLE), 1, outptr);
        }
        for (int k = 0; k < outPadding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);
    
    freeUserInput(&in);
    return 0;
}