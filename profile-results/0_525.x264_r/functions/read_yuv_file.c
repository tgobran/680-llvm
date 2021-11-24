/* Read a YUV420p file and convert it to RGB.
 * Raw YUV files have no header, so the image dimensions are taken
 * from parameters supplied on the command-line.  If there are none,
 * the the default of 1280x720 is used.
 */
#define clampval(dest, tmptype, min, max, val) \
    {                                   \
        tmptype clampedtmp = (val);     \
        if (clampedtmp < min) {         \
            dest = min;                 \
        } else if (clampedtmp > max) {  \
            dest = max;                 \
        } else {                        \
            dest = clampedtmp;          \
        }                               \
    }
void read_yuv_file (const char * fname, TargaImage * tga, params param) {
    int i, j, h, w, nb;
    unsigned char *buffer;
    YUV **pixels;
    int i2 = 0;

    /* Set default TGA header values */
    tga->idlength        = 0;
    tga->colormaptype    = 0;
    tga->datatypecode    = TRUECOLOR;
    tga->colormaporigin  = 0;
    tga->colormaplength  = 0;
    tga->colormapdepth   = 0;
    tga->x_origin        = 0;
    tga->y_origin        = 0;
    tga->width           = param.width;
    tga->height          = param.height;
    tga->bitsperpixel    = 24;
    tga->imagedescriptor = 0;
    tga->alphabits       = 0;

    /* Open the input yuv file */
    FILE *fin = fopen(fname, "rb");
    if (fin == NULL) {
        fprintf(stderr, "'%s': Could not open for read\n", fname);
        exit(-1);
    }

    /* Allocate space for the final RGB image data, the YUV input data, and
     * the buffer to hold the mishmash of data in the file.
     */
    check_malloc( tga->imagedata, Pixel **, (sizeof(Pixel *) * tga->height) );
    check_malloc( pixels, YUV **, tga->height * sizeof(YUV *) );
    check_malloc( buffer, unsigned char *, tga->width * sizeof(unsigned char) );
    for (h = 0; h < tga->height; ++h) {
        check_malloc( tga->imagedata[h], Pixel *, sizeof(Pixel) * tga->width );
        check_malloc( pixels[h], YUV *, tga->width * sizeof(YUV) );
    }

    /* Read in the Y, which are the first WxH bytes in the file */
    for (i = 0;i < tga->height; i++) {
        int nb = fread(buffer, 1, tga->width, fin);
        if (nb < tga->width) {
            fprintf(stderr, "%s: Error reading Y data: Only got %d of %d\n",
                    fname, nb, tga->width);
            exit(1);
        }
        for (j = 0; j < tga->width; j++) {
            pixels[i][j].y = buffer[j];
        }
    }

    /* Read in the U.  There's one value for every 2x2 block of pixels,
     * so there are WxH/4 bytes total
     */
    for (i = 0; i < tga->height; i++) {
        if (i % 2 == 0) {       /* Only read data every other row */
            nb = fread(buffer, 1, tga->width / 2, fin);
            if (nb < tga->width / 2) {
                fprintf(stderr, "%s: Error reading U data: Only got %d of %d\n",
                        fname, nb, tga->width / 2);
                exit(1);
            }
        }
        for (i2 = 0, j = 0; j < tga->width; ++i2, j += 2) {
            pixels[i][j    ].u = buffer[i2];
            pixels[i][j + 1].u = buffer[i2];
        }
    }

    /* Read in the V.  As with U, there's one value for every 2x2 block of
     * pixels, so there are WxH/4 bytes total
     */
    for (i = 0; i < tga->height; i++) {
        if (i % 2 == 0) {       /* Only read data every other row */
            nb = fread(buffer, 1, tga->width / 2, fin);
            if (nb < tga->width / 2) {
                fprintf(stderr, "%s: Error reading V data: Only got %d of %d\n",
                        fname, nb, tga->width / 2);
                exit(1);
            }
        }
        for (i2 = 0, j = 0; j < tga->width; ++i2, j += 2) {
            pixels[i][j    ].v = buffer[i2];
            pixels[i][j + 1].v = buffer[i2];
        }
    }

    /* Now do the YUV444 to RGB888 conversion */
    h=0;
    for (i = tga->height - 1; i >= 0; i--) {
        w=0;
        for (j = 0;j < tga->width; j++) {
            clampval( tga->imagedata[h][w].blue,  int, 16, 255,
                      1.164 * (pixels[i][j].y - 16) + 2.018 * (pixels[i][j].u - 128) );
            clampval( tga->imagedata[h][w].green, int, 16, 255,
                      1.164 * (pixels[i][j].y - 16) - 0.812 * (pixels[i][j].v - 128) - 0.391 * (pixels[i][j].u - 128) );
            clampval( tga->imagedata[h][w].red,   int, 16, 255,
                      1.164 * (pixels[i][j].y - 16) + 1.596 * (pixels[i][j].v - 128) );
            ++w;
        }
        ++h;
    }

    fclose(fin);

    for (h = 0; h < tga->height; ++h) {
        free(pixels[h]);
    }
    free(buffer);
    free(pixels);
}