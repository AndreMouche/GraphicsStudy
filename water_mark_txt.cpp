/*
*#ld g++ water_mark_txt.cpp `GraphicsMagick-config --cppflags --ldflags --libs`
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <magick/api.h>
#include <iostream>
using namespace std;

#define FONT_DEFAULT "./simsun.ttc"
#define COLOR_MAPPING "RGBA"

bool IsGIF(const Image * image)
{
        return (strcmp(image->magick, "GIF") == 0);
}

/**
 * set dissolve 
 * @param image 
 * @dissolve:0~100,0 means totally transparent while 100 means opa,q
 * */
MagickPassFail dissolveImage(Image *image,int dissolve){
    int y, x;
    register PixelPacket
        *q;
    for (y=0; y < (long) image->rows; y++)
    {
        q=GetImagePixels(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
            break;
        for (x=0; x < (long) image->columns; x++)
        {
            if(q->opacity != MaxRGB) {
                q->opacity=(Quantum)
                    (MaxRGB - ((MaxRGB-q->opacity)/100.0*dissolve));
            }
            q++;
        }
        if (!SyncImagePixels(image)){
            return MagickFail;
        }
    }
    return MagickPass;
}

/** 
 * Trim Image
 * remove transparent part around the Text
 * return image with text only
 */
Image * trimImage(Image *& image){
    if(IsGIF(image)) {
       return NULL;
    }
    int y, x;
    int sx,sy,ex,ey;
    register PixelPacket *q;
    RectangleInfo rect;
    ExceptionInfo exception;
    GetExceptionInfo(&exception);
    sx = image->columns;
    sy = image->rows;
    ex = 0;
    ey = 0;

    for (y=0; y < (long) image->rows; y++)
    {
        q=GetImagePixels(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
            break;
        for (x=0; x < (long) image->columns; x++)
        {
            if(q->opacity != MaxRGB) {
                sx = sx < x ? sx:x;
                sy = sy < y ? sy:y;
                ex = ex > x ? ex:x;
                ey = ey > y ? ey:y;                
            }
            q++;
        }
    }
    cout << "sx:" << sx << ",sy:" << sy << ",ex:" << ex << ",ey:" << ey << endl;
    rect.x = sx;
    rect.y = sy;
    rect.width = ex - sx;
    rect.height = ey - sy;
    cout << "x:" << rect.x << ",y:" << rect.y << ",rect.width:" << rect.width << ",rect.height:" << rect.height << endl;
    Image *cImage = CropImage(image, &rect, &exception);
    cout << "cImage clomnes:" << cImage->columns << ",rows:" << cImage->rows << endl;

    DestroyExceptionInfo(&exception);
    return cImage;
}

/**
 * Draw Text on the center of the image 
 *@param:image,image to annotate
 *@param:text,text to annotate
 *@param:font_pointsize,fontsize in point
 *@param:fill_color,color of font 
 *return true on success while false on failed
 */
bool DrawText(Image *& image, const char * text, const double font_pointsize,const char * fill_color) 
{
    cout << "text:" << text << endl;
    cout << "color:" << fill_color << endl;

    DrawContext draw_context;
    draw_context = DrawAllocateContext((DrawInfo*)NULL, image);
    DrawSetFillColorString(draw_context, fill_color);

    DrawSetTextAntialias(draw_context, 1);
    DrawSetFont(draw_context, FONT_DEFAULT);
    DrawSetFontSize(draw_context, font_pointsize);
    DrawSetGravity(draw_context, CenterGravity);
    DrawSetTextEncoding(draw_context, "UTF-8");
    DrawAnnotation(draw_context, 0, 0, (const unsigned char *)text);
    unsigned int ret = DrawRender(draw_context);
    DrawDestroyContext(draw_context);
    return ret;
}

/**
 * Constitute an transparent Image with nCloumns and nRows
 * @param:img,pointer to save the dest image 
 * @param:nColumns,columns of the dest image 
 * @param:nRows,rows of the dest image 
 * return true with transparent image in img,return false on failed.
 */
bool constituteNewImage(Image * & img, int nColumns, int nRows)
{
    ExceptionInfo exception;
    GetExceptionInfo(&exception);

    const size_t pixels_size = nColumns * nRows * sizeof(Quantum) * strlen(COLOR_MAPPING);
    Quantum * pixels = (Quantum *)MagickMalloc(pixels_size);
    memset((void *) pixels, 0, pixels_size);
    Image * canvasImage = ConstituteImage(nColumns, nRows, COLOR_MAPPING, CharPixel, pixels, &exception);
    //made it transparent
    SetImage(canvasImage,Quantum(MaxRGB));//must
    canvasImage->matte = true;//musta
    MagickFree(pixels);
    DestroyExceptionInfo(&exception);

    if (img != NULL)
        DestroyImageList(img);
    img = canvasImage;
    return true;
}


Image * GetTextDraw(Image *image, const char * text, 
        const double font_pointsize, const char * fill_color)
{
    Image  *maskImg = (Image *) NULL;
    Image  *textImg = NULL;
    if(!constituteNewImage(maskImg, image->columns, image->rows)) {   
        cout << "ConstituteNewImage maskImg failed" << endl;
        goto ERROR_HANDLE;
    }

    cout << "ConstituteNewImage maskImg success" << endl;
    if(!DrawText(maskImg, text, font_pointsize,fill_color)) {
        cout << "DrawText maskImg failed";
        goto ERROR_HANDLE;
    }
    if(maskImg == NULL) {
      cout << "qunidayede" << endl;
    }
    cout << "DrawText success" << endl;
    textImg =  trimImage(maskImg);
    if(textImg != NULL) {
        cout << "trim ok" << endl;
        cout << "columns:" << textImg->columns << ",y:" << textImg->rows << endl;
    }
    if(maskImg!=NULL) {
       DestroyImage(maskImg);
    }
    cout << "haqi!!!!" << endl;
    return textImg;
ERROR_HANDLE:
    cout << "~~~~~~~~~~~~~~~" << endl;
    if(maskImg != NULL) {
      DestroyImageList(maskImg);
    }
    return NULL;
}

/**
 *Composite Image 
 *@param image:source image 
 *@param logo:logo image 
 *@param gravity,
 *|NorthWestGravity |NorthGravity   |NorthEastGravity   |
 *|WestGravity      |CenterGravity  |EastGravity        |
 *|SouthEastGravity |SouthGravity   |SouthEastGravity   |
 *@param dx:offset on x
 *@param dy:offset on y
 * return true on success,false on failed 
 */
bool compositeImage(Image *image,Image *logo,GravityType gravity,long dx,long dy) {
   int x_offset,y_offset;

   switch(gravity){
       case NorthWestGravity:{
           x_offset = dx;
           y_offset = dy;
           break;
        }
       case NorthGravity:{
           x_offset = image->columns/2 - logo->columns/2 + dx;
           y_offset = dy;
           break;
       }
       case NorthEastGravity: {
           x_offset = image->columns-dx-logo->columns;
           y_offset = dy;
           break;
       }
       case WestGravity:{
           x_offset = dx;
           y_offset = image->rows/2 - logo->rows/2 + dy;
           break;
       }
       case CenterGravity: {
          x_offset = image->columns/2 - logo->columns/2 + dx;
          y_offset = image->rows/2 - logo->rows/2 + dy;
          break;
       }
       case EastGravity:{
          x_offset = image->columns - dx - logo->columns;       
          y_offset = image->rows/2 - logo->rows/2 + dy;
          break;
       }
       case SouthWestGravity:{
          x_offset = dx;
          y_offset = image->rows - logo->rows - dy;
          break;
       }

       case SouthGravity:
           x_offset = image->columns/2 - logo->columns/2 + dx;
           y_offset = image->rows - logo->rows - dy;
           break;
       
       case SouthEastGravity:{
           x_offset = image->columns - dx - logo->columns;
           y_offset = image->rows - logo->rows - dy;
           break;
       }
       default:{
           cout << "Illegal GravityType:" << gravity;      
           break;
       }
   }

    if(CompositeImage(image, AtopCompositeOp,logo, x_offset, y_offset) != 1) {
          return false;
    }
    return true;
}

bool DrawTextTransparent(Image * image, const char * text, 
        const double font_pointsize, 
        const char * fill_color, const GravityType gravity,
        int dissolve,
        const double x, const double y)
{

   
    PixelPacket textColor;
    cout << "DrawTextTransparent start";
    Image * composite_image = NULL;
    Image * mask_image = NULL;
    
     ExceptionInfo exception;
    GetExceptionInfo(&exception);
    mask_image = GetTextDraw(image,text,font_pointsize,fill_color);
    if(mask_image == NULL) {
        cout << "God";
        goto ERROR_HANDLE;
    }
    cout << "DrawText success";
    
    if(dissolveImage(mask_image,dissolve)== MagickFail) {
       cout << "dissolve image failed" << endl;
       goto ERROR_HANDLE;
    }
    
    //if(CompositeImage(image, AtopCompositeOp, mask_image, 0, 0) != 1) {
    if(!compositeImage(image,mask_image,gravity,x,y)){
        cout << "CompositeImage img composite_image failed";
        goto ERROR_HANDLE;
    }
    cout << "CompositeImage img composite_image success";

    DestroyImageList(composite_image);
    DestroyImageList(mask_image);
    return true;
ERROR_HANDLE:
    if (composite_image != NULL)
        DestroyImageList(composite_image);
    if (mask_image != NULL)
        DestroyImageList(mask_image);
    DestroyExceptionInfo(&exception);
    return false;
}

bool ImgRead(string path,Image * & img,ImageInfo * imageInfo){
    bool ret = true;
    ExceptionInfo exception;
    GetExceptionInfo(&exception);

    imageInfo = CloneImageInfo((ImageInfo *) NULL);

    strcpy(imageInfo->filename,path.c_str());
 
    img = ReadImage(imageInfo, &exception);
    if ((exception.severity >= FatalErrorException) 
        || (img == NULL)) {
         ret = false;
    }

    DestroyExceptionInfo(&exception);
    
    return ret;
}

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
 
    if(!DrawTextTransparent(image,"您好, World!",200,"white",SouthEastGravity,50,30,0)) {
       goto program_exit;
    }


    (void) strcpy(image->filename, outfile);

    
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

