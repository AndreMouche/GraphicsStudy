/* Sample 
 * 使用graphmagick提取图片基本信息
 * build g++ image_info.cpp `GraphicsMagick-config --cppflags --ldflags --libs`
 *
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <magick/api.h>
#include<iostream>
using namespace std;

int main ( int argc, char **argv )
{
    Image
        *image = (Image *) NULL;

    char
        infile[MaxTextExtent];
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

    if (argc != 2)
    {
        (void) fprintf ( stderr, "Usage: %s infile\n", argv[0] );
        (void) fflush(stderr);
        exit_status = 1;
        goto program_exit;
    }

    (void) strncpy(infile, argv[arg], MaxTextExtent-1 );

    (void) strcpy(imageInfo->filename, infile);

    if((image = PingImage(imageInfo,&exception)) == NULL) {
        printf("PingImage fail, path = %s",infile);
        exit_status = -1;
        goto program_exit;
    }

    //获取图片基本信息
    cout << "image Type:" << image->magick << endl;
    cout << "image width:" << image->columns << endl;
    cout << "image height:" << image->rows << endl;
    cout << "Image colorspace:" << image->colorspace << endl;

    /*switch(image->colorspace){


      }*/




program_exit:

    if (image != (Image *) NULL)
        DestroyImage(image);

    if (imageInfo != (ImageInfo *) NULL)
        DestroyImageInfo(imageInfo);
    DestroyMagick();

    return exit_status;
}
