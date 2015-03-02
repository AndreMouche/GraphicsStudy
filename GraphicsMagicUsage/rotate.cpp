/*
 * build 
 * g++ rotate.cpp `GraphicsMagick-config --cppflags --ldflags --libs`
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
    double degree = 120; //旋转角度
   
    PixelPacket background_color = {204,255,153, TransparentOpacity};
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
//   image->background_color= background_color; //转完后的图片空白部分颜色
    //Rotate image 
    image = RotateImage(image,degree,&exception);   
 
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
