int computeSSIM(params param) {
    TargaImage *img1     = param.img1;
    TargaImage *img1luma = param.img1luma;
    TargaImage *img2     = param.img2;
    TargaImage *img2luma = param.img2luma;
    TargaImage *diff     = param.imgdiff;

    unsigned int height   = img1->height;
    unsigned int width    = img1->width;
    if (width != img2->width || height != img2->height) {
        fprintf(stderr, "ERROR: Image sizes don't match (%ux%u vs %ux%u)\n",
                width, height,
                img2->width, img2->height);
        return 0; /* Not -1; This is a VE, not an RE */
    }

    /* The ACTUAL number of bits per pixel isn't necessarily what's in the
     * bitsperpixel field; Targa-16 images have 15 bits of color data per
     * pixel, and Targa-32 images have 24.
     */
    int bpp = (img1->bitsperpixel == 16) ? 15 : (img1->bitsperpixel == 32) ? 24 : img1->bitsperpixel;

    /* For images, SSIM is applied only to luma, and our luma calculations
     * result in values [0,255], so there's effectively 8 bits of dynamic
     * range, regardless of how many bits per pixel the original images have.
     */
    /* c = (k * L)^2 */
    double c1 = 6.5025;         /* k1 = 0.01 */
    double c2 = 58.5255;        /* k2 = 0.03 */

    double       sum      = 0.0;
    double       minssim  = 1.0;
    unsigned int cnt      = 0;
    unsigned int numbelow = 0;

    /* Compute the SSIM of each 8x8 pixel area for this image against the
     * ref image.
     */
    double *i1_lumas, *i2_lumas;
    check_malloc( i1_lumas, double *, sizeof(double) * 8 * 8 );
    check_malloc( i2_lumas, double *, sizeof(double) * 8 * 8 );
    for (unsigned int i = 0; i < height; i += 8 ) {
        for (unsigned int j = 0; j < width; j += 8 ) {
            unsigned int nx = ((i + 8) > height) ? (height - i) : 8;
            unsigned int ny = ((j + 8) > width) ? (width - j) : 8;
            unsigned int n = 0;

            /* The RGBtoLuma macro will compute luma (brightness) value for
             * each pixel using the ITU-R Rec. 601 standard-def digital TV
             * formula.
             * http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC11
             */
#define RGBto601Luma(pixel) (  (0.299 * (pixel).red)   \
                             + (0.587 * (pixel).green) \
                             + (0.114 * (pixel).blue)  \
                            )

            /* The RGBtoLuma macro will compute luma (brightness) value for
             * each pixel using the ITU-R Rec. 709 high-def digital TV
             * formula.
             * http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC9
             */
#define RGBto709Luma(pixel) (  (0.2126 * (pixel).red)   \
                             + (0.7152 * (pixel).green) \
                             + (0.0722 * (pixel).blue)  \
                            )

            /* The RGBtoP macro compute brightness value for each pixel using
             * the "HSP299" formula from
             * http://alienryderflex.com/hsp.html
             */
#define RGBtoP(pixel)   sqrt( \
                             (0.299 * (pixel).red   * (pixel).red)   \
                           + (0.587 * (pixel).green * (pixel).green) \
                           + (0.114 * (pixel).blue  * (pixel).blue)  \
                        )

            for (unsigned int ix = 0; ix < nx; ix++ ) {
                for (unsigned int jx = 0; jx < ny; jx++ ) {
                    if (bpp == 8) {
                        /* The scale of the luma value is [0,255], so just use the
                         * red (gray) channel directly.
                         */
                        i1_lumas[n] = img1->imagedata[i + ix][j + jx].red;
                        i2_lumas[n] = img2->imagedata[i + ix][j + jx].red;
                    } else if (param.conversion == REC709) {
                        i1_lumas[n] = RGBto709Luma(img1->imagedata[i + ix][j + jx]);
                        i2_lumas[n] = RGBto709Luma(img2->imagedata[i + ix][j + jx]);
                    } else if (param.conversion == REC601) {
                        i1_lumas[n] = RGBto601Luma(img1->imagedata[i + ix][j + jx]);
                        i2_lumas[n] = RGBto601Luma(img2->imagedata[i + ix][j + jx]);
                    } else if (param.conversion == HSP) {
                        i1_lumas[n] = RGBtoP(img1->imagedata[i + ix][j + jx]);
                        i2_lumas[n] = RGBtoP(img2->imagedata[i + ix][j + jx]);
                    } else {
                        fprintf(stderr, "ERROR: Unknown luma conversion method '%d' specified\n",
                                (int)param.conversion);
                        exit(1);
                    }
                    if (img1luma != NULL) {
                        img1luma->imagedata[i + ix][j + jx].red = (unsigned char)i1_lumas[n];
                    }
                    if (img2luma != NULL) {
                        img2luma->imagedata[i + ix][j + jx].red = (unsigned char)i2_lumas[n];
                    }
                    if (param.debug > 3) {
                        fprintf(stderr, "(%5d,%5d) (%3d, %3d, %3d) => %0.5g (%u)\n",
                                i+ix, j+jx,
                                img1->imagedata[i+ix][j+jx].red,
                                img1->imagedata[i+ix][j+jx].green,
                                img1->imagedata[i+ix][j+jx].blue,
                                i1_lumas[n],
                                img1luma ? img1luma->imagedata[i + ix][j + jx].red : 0);
                        fprintf(stderr, "              (%3d, %3d, %3d) => %0.5g (%u)\n",
                                img2->imagedata[i+ix][j+jx].red,
                                img2->imagedata[i+ix][j+jx].green,
                                img2->imagedata[i+ix][j+jx].blue,
                                i2_lumas[n],
                                img2luma ? img2luma->imagedata[i + ix][j + jx].red : 0);
                    }
                    n++;
                }
            }
            double i1_mean = mean(i1_lumas, n);
            double i2_mean = mean(i2_lumas, n);
            double i1_var  = variance(i1_lumas, i1_mean, n);
            double i2_var  = variance(i2_lumas, i2_mean, n);
            double covar   = covariance(i1_lumas, i2_lumas, i1_mean, i2_mean, n);
            double ssim    = ((2 * i1_mean * i2_mean + c1) * (2 * covar + c2)) /
                             ((i1_mean * i1_mean + i2_mean * i2_mean + c1) * (i1_var + i2_var + c2));
            ++cnt;
            sum += ssim;

            if (!param.avg_only) {
                printf("(%d,%d)  SSIM = %10.9f\n", j, i, ssim);
                if (param.debug)
                    fflush(stdout);
            }
            if (ssim < minssim) {
                minssim = ssim;
            }
            if (param.numbuckets > 0) {
                int bucket = (ssim * ((float)param.numbuckets / 2)) + (param.numbuckets / 2);
                /* 1.0 SSIM gets its own bucket, so no adjustment needed */
                assert(bucket <= (int)param.numbuckets && bucket >= 0);
                param.buckets[bucket]++;
            }
            if (param.threshold > 0.0 && ssim < param.threshold) {
                ++numbelow;
            }

            if (diff != NULL) {
                for (unsigned int ix = 0; ix < nx; ix++ ) {
                    for (unsigned int jx = 0; jx < ny; jx++ ) {
                        unsigned char luma = img1luma->imagedata[i + ix][j + jx].red;
                        diff->imagedata[i + ix][j + jx].red   = luma;
                        if (ssim >= 1) {
                            /* Identical, so no coloring needed.  Copy the luma
                             * data into all the channels to make a greyscale.
                             */
                            diff->imagedata[i + ix][j + jx].blue  =
                            diff->imagedata[i + ix][j + jx].green = luma;
                        } else {
                            /* Scale the SSIM to [0,luma-16] and put it in
                             * the blue and green channels.  As SSIM approaches
                             * -1, the pixels will appear more and more red.
                             */
                            if (luma < 16) {
                                luma = 16;
                            }
                            unsigned char reduction = (luma - 16) * ((ssim + 1) / 2);
                            diff->imagedata[i + ix][j + jx].blue  =
                            diff->imagedata[i + ix][j + jx].green = reduction;
                        }
                    }
                }
            }

            if (param.debug > 1) {
                fprintf(stderr, "means:\timg1: %0.40g\n\timg2: %0.40g\n",
                        i1_mean, i2_mean);
                fprintf(stderr, "variances:\timg1: %0.40g\n\t\timg2: %0.40g\n",
                        i1_var, i2_var);
                fprintf(stderr, "covariance: %0.40g\n", covar);
                fprintf(stderr, "(%d,%d) ssim: %0.40g (%u)\n", j, i, ssim,
                        diff ? diff->imagedata[i][j].red : 0);
            }

        }
    }
    free(i1_lumas);
    free(i2_lumas);

    FILE *tableout = stderr;
    unsigned int maxthresh = param.maxthresh;
    /* The bar for passing should be the lesser of maxthresh and
     * maxthreshpct*cnt, if both are set.
     */
    if (param.maxthreshpct > 0) {
        if (param.maxthresh <= 0
                ||
                param.maxthresh > (param.maxthreshpct * cnt)) {
            maxthresh = param.maxthreshpct * cnt;
        }
    }
#define dumptable(stream, format, ...) \
    fprintf(stream, format, __VA_ARGS__);     \
    if (stream != stderr) {                   \
        if (param.debug)                      \
            fflush(stdout);                   \
        fprintf(stderr, format, __VA_ARGS__); \
    }
    if (numbelow > maxthresh) {
        printf("%s ERROR:\n", param.file1);
        printf("The maximum number of SSIM below the threshold has been reached. \n");
        tableout = stdout;
    } else {
        dumptable(tableout, "%s: INFO:\n", param.file1);
    }

    if (param.threshold > 0.0) {
        dumptable(tableout, "  Blocks below threshold: %d blocks of %d allowed (%d total)\n", numbelow, maxthresh, cnt);
    }
    if (cnt > 0) {
        dumptable(tableout, "                AVG SSIM: %10.9f\n", sum/cnt);
    }
    dumptable(tableout, "                MIN SSIM: %10.9f\n", minssim);
    param.printbuckets = 1;
    if (param.avg_only) {
        if (cnt > 0) {
            printf("%s: AVG SSIM = %10.9f\n", param.file1, sum/cnt);
        } else {
            printf("%s: ERROR: Count is zero.\n", param.file1);
        }
    }
    if (param.debug)
        fflush(stdout);

    if (param.printbuckets) {
        dumptable(tableout, "       SSIM distribution:%c", '\n');
        dumptable(tableout, "         [        % 4.3f]: %d\n", 1.0, param.buckets[param.numbuckets]);
        for (int i = param.numbuckets - 1; i >= 0; --i) {
            float r1 = (((float) i - (param.numbuckets / 2))     / (float)param.numbuckets) * 2;
            float r2 = (((float) i - (param.numbuckets / 2) + 1) / (float)param.numbuckets) * 2;
            dumptable(tableout, "         [% 4.3f, % 4.3f): %d\n", r1, r2, param.buckets[i]);
        }
    }
    return 0;
}