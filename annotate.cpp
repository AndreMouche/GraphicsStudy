/*
 * build g++ convert.cpp `GraphicsMagick-config --cppflags --ldflags --libs`
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <magick/api.h>

int main ( int argc, char **argv )
{
    Image
        *image = (Image *) NULL;

    char
        infile[MaxTextExtent],
        outfile[MaxTextExtent],
        annotateText[100] = "HelloWorld",
        gemery[100]="+10+10",
        fontFile[MaxTextExtent]= "./simsun.ttc";

    int
        arg = 1,
            exit_status = 0;

    ImageInfo
        *imageInfo;

    DrawInfo 
        *drawInfo;

    ExceptionInfo
        exception;



    InitializeMagick(NULL);
    imageInfo = CloneImageInfo(0);
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

    //fontFile
    if((drawInfo = CloneDrawInfo(imageInfo, NULL)) == NULL) {
        exit_status = 1;
        goto program_exit;
    }

    drawInfo->font = (char*)fontFile;
    drawInfo->pointsize = 15;
    drawInfo->gravity = SouthEastGravity;
    drawInfo->stroke.opacity = (Quantum) (((double) MaxRGB*(1.0 - 0.0))+0.5);
    drawInfo->opacity = (Quantum) (((double) MaxRGB*(1.0 - 0.0))+0.5);
    drawInfo->stroke_antialias = 1;
    drawInfo->text_antialias = 1;
    drawInfo->weight = 100;
    drawInfo->geometry = gemery;

    QueryColorDatabase("#000000", &(drawInfo->fill), &exception);
    drawInfo->text = (char*)annotateText;
    (void) strcpy(imageInfo->filename, infile);
    image = ReadImage(imageInfo, &exception);
    if (image == (Image *) NULL)
    {
        CatchException(&exception);
        exit_status = 1;
        goto program_exit;
    }

    printf("pre annotate\n");
    AnnotateImage(image, drawInfo);
    printf("finish annotate\n");
    (void) strcpy(image->filename, outfile);
    if (!WriteImage (imageInfo,image))
        //if (!WriteWEBPImage(imageInfo,image)
    {
        CatchException(&image->exception);
        exit_status = 1;
        goto program_exit;
    }

program_exit:

    if (image != (Image *) NULL)
        DestroyImage(image);

    if (imageInfo != (ImageInfo *) NULL)
        DestroyImageInfo(imageInfo);


    DestroyMagick();

    return exit_status;
}
