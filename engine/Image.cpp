#include <string.h>
#include <png.h>

#include "Common.h"
#include "OpenGL.h"
#include "Image.h"


bool CreateImage( Image* image, int width, int height, int bpp )
{
    memset(image, 0, sizeof(Image));

    image->width = width;
    image->height = height;
    image->bpp = bpp;

    switch(image->bpp)
    {
        case 1:
            image->format = GL_LUMINANCE;
            break;

        case 2:
            image->format = GL_LUMINANCE_ALPHA;
            break;

        case 3:
            image->format = GL_RGB;
            break;

        case 4:
            image->format = GL_RGBA;
            break;

        default:
            Error("Can't create image (Unknown BPP -> %d)", image->bpp);
            return false;
    }

    image->type = GL_UNSIGNED_BYTE;

    image->data = new char[width*height*bpp];

    return true;
}


bool LoadPngImage( Image* image, const char* file )
{
    //header for testing if it is a png
    png_byte header[8];

    //open file as binary
    FILE* fp = fopen(file, "rb");
    if(!fp) return 0;

    //read the header
    size_t read = fread(header, 1, 8, fp);
    if(read != 8 && ferror(fp))
        return false;

    //test if png
    int is_png = !png_sig_cmp(header, 0, 8);
    if(!is_png)
    {
        fclose(fp);
        return false;
    }

    //create png struct
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
    {
        fclose(fp);
        return false;
    }

    //create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
        fclose(fp);
        return false;
    }

    //create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if(!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return false;
    }

    //png error stuff, not sure libpng man suggests this.
    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return false;
    }

    //init png reading
    png_init_io(png_ptr, fp);

    //let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    //variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 twidth, theight;

    // Only 8-bit!
    png_set_strip_16(png_ptr);

    // OGL doesn't likes indexed data. (Extensions?)
    png_set_expand(png_ptr);

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    //update width and height based on png info
    image->width = int(twidth);
    image->height = int(theight);

    switch(png_get_color_type(png_ptr, info_ptr))
    {
        case PNG_COLOR_TYPE_GRAY:       image->bpp = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: image->bpp = 2; break;
        case PNG_COLOR_TYPE_RGB:        image->bpp = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  image->bpp = 4; break;
        case PNG_COLOR_TYPE_PALETTE:
        {
            image->bpp = 3;
            int trans = 0;
            png_get_tRNS(png_ptr, info_ptr, NULL, &trans, NULL);
            if(trans > 0)
                image->bpp += 1;
        } break;
        default: image->bpp = 0;
    }

    image->bpp *= png_get_bit_depth(png_ptr, info_ptr) / 8;

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte* image_data = new png_byte[rowbytes * theight];
    if(!image_data)
    {
        //clean up memory and close stuff
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return false;
    }

    //row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep* row_pointers = new png_bytep[theight];
    if(!row_pointers)
    {
        //clean up memory and close stuff
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        delete[] image_data;
        fclose(fp);
        return false;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for(png_uint_32 i = 0; i < theight; ++i)
        row_pointers[theight - 1 - i] = image_data + i * rowbytes;

    //read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    image->data = reinterpret_cast<char*>(image_data);

    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    delete[] row_pointers;
    fclose(fp);

    return true;
}


bool LoadImage( Image* image, const char* file )
{
    if(!LoadPngImage(image, file))
    {
        Error("Can't load '%s'", file);
        return false;
    }

    switch(image->bpp)
    {
        case 1:
            image->format = GL_LUMINANCE;
            break;

        case 2:
            image->format = GL_LUMINANCE_ALPHA;
            break;

        case 3:
            image->format = GL_RGB;
            break;

        case 4:
            image->format = GL_RGBA;
            break;

        default:
            Error("Can't load '%s' (Unknown BPP -> %d)", file, image->bpp);
            delete[] image->data;
            return false;
    }

    image->type = GL_UNSIGNED_BYTE;

    return true;
}


void FreeImage( const Image* image )
{
    if(image->data)
        delete[] image->data;
}