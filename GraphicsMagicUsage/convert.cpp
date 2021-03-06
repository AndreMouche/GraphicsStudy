/*
 * build g++ convert.cpp `GraphicsMagick-config --cppflags --ldflags --libs`
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <magick/api.h>
#include <iostream>
using namespace std;

int main ( int argc, char **argv )
{
    Image
        *image = (Image *) NULL;

    char
        infile[MaxTextExtent],
        outfile[MaxTextExtent];

    int
        arg = 1,
            exit_status = 0;

    ImageInfo
        *imageInfo;

    ExceptionInfo
        exception;

    InitializeMagick(NULL);
    imageInfo=CloneImageInfo(0);
    GetExceptionInfo(&exception);

    if (argc != 3)
    {
        (void) fprintf ( stderr, "Usage: %s infile outfile\n", argv[0] );
        (void) fflush(stderr);
        exit_status = 1;
        goto program_exit;
    }

    (void) strncpy(infile, argv[arg], MaxTextExtent-1 );
    arg++;
    (void) strncpy(outfile, argv[arg], MaxTextExtent-1 );

    (void) strcpy(imageInfo->filename, infile);
    image = ReadImage(imageInfo, &exception);
    if (image == (Image *) NULL)
    {
        CatchException(&exception);
        exit_status = 1;
        goto program_exit;
    }

    (void) strcpy(image->filename, outfile);

    //设置图片渐进方式，
    // 其中参数[PartitionInterlace,PlaneInterlace]未实现，
    // NoneInterlace为关闭渐进方式，目前仅LineInterlace可用
    imageInfo->interlace = LineInterlace;
    
    //保存图片
    if (!WriteImage (imageInfo,image)) {
        CatchException(&image->exception);
        exit_status = 1;
        goto program_exit;
    }

    //cout << "interlace:" << image->interlace << endl;

program_exit:

    if (image != (Image *) NULL)
        DestroyImage(image);

    if (imageInfo != (ImageInfo *) NULL)
        DestroyImageInfo(imageInfo);
    DestroyMagick();

    return exit_status;
}
