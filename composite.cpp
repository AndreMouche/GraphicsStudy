/**
 * build g++ composite.cpp `GraphicsMagick-config --cppflags --ldflags --libs`
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <magick/api.h>
#include <iostream>
using namespace std;
/**
 * 设置图片透明度
 * */
MagickPassFail dissolveImage(Image *image,int dissolve){
    int y, x;
    register PixelPacket
        *q;
    if (!image->matte)
        SetImageOpacity(image,OpaqueOpacity);

    for (y=0; y < (long) image->rows; y++)
    {
        q=GetImagePixels(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
            break;
        for (x=0; x < (long) image->columns; x++)
        {
            q->opacity=(Quantum)
                ((((unsigned long) MaxRGB-q->opacity)*dissolve)/100.0);
            q++;
        }
        if (!SyncImagePixels(image)){
            return MagickFail;
        }
    }
    return MagickPass;
}

int main ( int argc, char **argv )
{
    Image
        *canvas_image = (Image *) NULL,
        *composite_image = (Image *) NULL;

    char
        canvasimage[MaxTextExtent],
        compositeimage[MaxTextExtent],
        outfile[MaxTextExtent];

    int
        arg = 1,
            exit_status = 0;

    long
        xoffset = 0,
                yoffset = 0;

    CompositeOperator op = AtopCompositeOp;
    ImageInfo
        *imageInfo;

    ExceptionInfo
        exception;
    MagickPassFail pass;   
    InitializeMagick(NULL);
    imageInfo=CloneImageInfo(0);
    GetExceptionInfo(&exception);

    if (argc != 4)
    {
        (void) fprintf ( stderr, "Usage: %s canvas_image,compositeimage, outfile\n", argv[0] );
        (void) fflush(stderr);
        exit_status = 1;
        goto program_exit;
    }

    (void) strncpy(canvasimage, argv[arg], MaxTextExtent-1 );
    arg++;
    (void) strncpy(compositeimage, argv[arg], MaxTextExtent-1 );
    arg++;
    (void) strncpy(outfile, argv[arg], MaxTextExtent-1 );

    //读入画布
    (void) strcpy(imageInfo->filename, canvasimage);

    if((Image *)NULL ==  (canvas_image = ReadImage(imageInfo, &exception))) {
        CatchException(&exception);
        exit_status = 1;
        goto program_exit;
    }

    //水印图位置
    xoffset = long(canvas_image->columns*3.0/4);
    yoffset = long(canvas_image->rows*3.0/4);

    //读入水印图
    (void) strcpy(imageInfo->filename, compositeimage);
    if((Image *)NULL ==  (composite_image = ReadImage(imageInfo, &exception))) {
        CatchException(&exception);
        exit_status = 1;
        goto program_exit;
    }
 
    //设置水印透明度，disslove在0-100之间
    pass = dissolveImage(composite_image,70);
    if (pass == MagickFail) {
        CatchException(&exception);
        exit_status = 1;
        goto program_exit;
    }

    pass = CompositeImage(canvas_image,op,composite_image,xoffset,yoffset);
    if (pass == MagickFail) {
        CatchException(&exception);
        exit_status = 1;
        goto program_exit;
    }

    (void) strcpy(canvas_image->filename, outfile);
    if (!WriteImage (imageInfo,canvas_image))
    {
        CatchException(&canvas_image->exception);
        exit_status = 1;
        goto program_exit;
    }
    printf("success\n");
program_exit:

    if (canvas_image != (Image *) NULL)
        DestroyImage(canvas_image);

    if (composite_image != (Image *) NULL)
        DestroyImage(composite_image);

    if (imageInfo != (ImageInfo *) NULL)
        DestroyImageInfo(imageInfo);
    DestroyMagick();
    DestroyExceptionInfo(&exception);
    return exit_status;
}
