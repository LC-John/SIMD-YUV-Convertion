#include "convert.h"
#include <cstdio>
#include <time.h>
#include <cstring>
using namespace std;

void reset_progressbar(char* buf, int* ptr)
{
    *ptr = 1;
    buf[0] = '[';
    buf[20 + 2] = ']';
    memset(buf+1, '-', 21);
    buf[20 + 3] = '\0';
}

int main(int argc, char *argv[])
{
    double m_time_yuv2rgb = 0, m_time_merge = 0, m_time_rgb2yuv = 0;
    double t1_time_yuv2argb = 0, t1_time_argb2rgb = 0, t1_time_rgb2yuv = 0;
    double t2_time_yuv2argb = 0, t2_time_argb2rgb = 0, t2_time_rgb2yuv = 0;
    char progress_buf[100];
    int curr_progress = 1;
    clock_t start, end;
    SIMD_MODE mode = SIMD_NO;

    if (argc < 6)
        return -1;

    if (argc >= 7)
    {
        if (strncmp(argv[6], "-no", 3) == 0)
            mode = SIMD_NO;
        else if (strncmp(argv[6], "-mmx", 4) == 0)
            mode = SIMD_MMX;
        else if (strncmp(argv[6], "-sse", 4) == 0)
            mode = SIMD_SSE;
        else if (strncmp(argv[6], "-avx", 4) == 0)
            mode = SIMD_AVX;
        else
            return -2;
    }

    printf("\nMerging <%s> and <%s>\n", argv[1], argv[2]);
    if (load_yuv420_2(argv[1], argv[2]) < 0)
    {
        printf("Cannot load figures from <%s> or <%s>, abort!\n", argv[1], argv[2]);
        return -2;
    }
    //printf("  Figures <%s> and <%s> loaded!\n", argv[1], argv[2]);
    reset_progressbar(progress_buf, &curr_progress);
    for (int i = 0; i < n_frame; i++)
    {
        if (i % (n_frame / 20) == 0)
            progress_buf[curr_progress++] = '>';
        printf ("  %s [%d%%]  frame = %d\r",
                progress_buf, (int)((float)(i+1)/(float)n_frame*100), i+1);
        fflush(stdout);

        start = clock();
        yuv420_2_rgb888_2(mode);
        end = clock();
        m_time_yuv2rgb += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        merge_rgb888(3 + 3 * i, mode);
        end = clock();
        m_time_merge += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        rgb888_2_yuv420(i, mode);
        end = clock();
        m_time_rgb2yuv += (double)(end - start) / CLOCKS_PER_SEC;
    }
    printf("\n");
    if (store_yuv420_vedio(argv[5]) < 0)
    {
        printf("Cannot save vedio to <%s>, abort!\n", argv[5]);
        return -3;
    }
    //printf("  Merged vedio <%s> saved!\n", argv[5]);
    //printf("Merge <%s> and <%s> done!\n", argv[1], argv[2]);

    printf("\nTransforming <%s>\n", argv[1]);
    if (load_yuv420(argv[1]) < 0)
    {
        printf("Caanot load figure from <%s>, abort!\n", argv[1]);
        return -1;
    }
    //printf("  Figure <%s> loaded!\n", argv[1]);
    reset_progressbar(progress_buf, &curr_progress);
    for (int i = 0; i < n_frame; i++)
    {
        if (i % (n_frame / 20) == 0)
            progress_buf[curr_progress++] = '>';
        printf ("  %s [%d%%]  frame = %d\r",
                progress_buf, (int)((float)(i+1)/(float)n_frame*100), i+1);
        fflush(stdout);

        start = clock();
        yuv420_2_argb8888(3 + 3 * i, mode);
        end = clock();
        t1_time_yuv2argb += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        argb8888_2_rgb888(mode);
        end = clock();
        t1_time_argb2rgb += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        rgb888_2_yuv420(i, mode);
        end = clock();
        t1_time_rgb2yuv += (double)(end - start) / CLOCKS_PER_SEC;
    }
    printf("\n");
    if (store_yuv420_vedio(argv[3]) < 0)
    {
        printf("Caanot save vedio to <%s>, abort!\n", argv[3]);
        return -2;
    }
    //printf("  Transformed vedio <%s> saved!\n", argv[3]);
    //printf("Transforming <%s> done!\n", argv[1]);

    printf("\nTransforming <%s>\n", argv[2]);
    if (load_yuv420(argv[2]) < 0)
    {
        printf("Cannot load figure from <%s>, abort!\n", argv[2]);
        return -1;
    }
    //printf("  Figure <%s> loaded!\n", argv[2]);
    reset_progressbar(progress_buf, &curr_progress);
    for (int i = 0; i < n_frame; i++)
    {
        if (i % (n_frame / 20) == 0)
            progress_buf[curr_progress++] = '>';
        printf ("  %s [%d%%]  frame = %d\r",
                progress_buf, (int)((float)(i+1)/(float)n_frame*100), i+1);
        fflush(stdout);

        start = clock();
        yuv420_2_argb8888(3 + 3 * i, mode);
        end = clock();
        t2_time_yuv2argb += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        argb8888_2_rgb888(mode);
        end = clock();
        t2_time_argb2rgb += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        rgb888_2_yuv420(i, mode);
        end = clock();
        t2_time_rgb2yuv += (double)(end - start) / CLOCKS_PER_SEC;
    }
    printf("\n");
    if (store_yuv420_vedio(argv[4]) < 0)
    {
        printf ("Cannot save figure to <%s>, abort!\n", argv[4]);
        return -2;
    }
    //printf("  Transformed vedio <%s]> saved!\n", argv[4]);
    //printf("Transforming <%s> done!\n", argv[2]);

    printf("\n\nRESULT:\n");
    printf("Merge <%s> and <%s>:\n", argv[1], argv[2]);
    printf("  Total YUV420 to RGB888 time cost = %f\n", m_time_yuv2rgb);
    printf("  Total RGB888 merging time cost = %f\n", m_time_merge);
    printf("  Total RGB888 to YUV420 time cost = %f\n", m_time_rgb2yuv);
    printf("  Total time cost = %f\n", m_time_yuv2rgb+m_time_rgb2yuv+m_time_merge);
    printf("Transform <%s>:\n", argv[1]);
    printf("  Total YUV420 to ARGB8888 time cost = %f\n", t1_time_yuv2argb);
    printf("  Total ARGB8888 to RGB888 time cost = %f\n", t1_time_argb2rgb);
    printf("  Total RGB888 to YUV420 time cost = %f\n", t1_time_rgb2yuv);
    printf("  Total time cost = %f\n", t1_time_argb2rgb+t1_time_rgb2yuv+t1_time_yuv2argb);
    printf("Transform <%s>:\n", argv[2]);
    printf("  Total YUV420 to ARGB8888 time cost = %f\n", t2_time_yuv2argb);
    printf("  Total ARGB8888 to RGB888 time cost = %f\n", t2_time_argb2rgb);
    printf("  Total RGB888 to YUV420 time cost = %f\n", t2_time_rgb2yuv);
    printf("  Total time cost = %f\n", t2_time_argb2rgb+t2_time_rgb2yuv+t2_time_yuv2argb);
    printf("\n\n");

    return 0;
}
