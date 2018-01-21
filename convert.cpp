#include "convert.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std;

unsigned char y[h][w];
unsigned char u[h/2][w/2];
unsigned char v[h/2][w/2];
unsigned char argb[h][w][4];
unsigned char rgb[h][w][3];
unsigned char _y[n_frame][h][w];
unsigned char _u[n_frame][h/2][w/2];
unsigned char _v[n_frame][h/2][w/2];

unsigned char rgb2[2][h][w][3];
unsigned char y2[2][h][w];
unsigned char u2[2][h/2][w/2];
unsigned char v2[2][h/2][w/2];

int load_yuv420(char *a_name)
{
    FILE *file = NULL;
    if ((file = fopen(a_name, "r")) == NULL)
        return -1;

    fseek(file, 0, 0);
    fread(y, 1, w*h, file);
    fread(u, 1, w/2*h/2, file);
    fread(v, 1, w/2*h/2, file);

    fclose(file);
    return 0;
}

int store_yuv420(char *a_name)
{
    FILE *file = fopen(a_name, "w");
    if (file == NULL)
        return -1;

    fwrite(y, 1, w*h, file);
    fwrite(u, 1, w/2*h/2, file);
    fwrite(v, 1, w/2*h/2, file);

    fclose(file);
    return 0;
}

int yuv420_2_argb8888(unsigned char alpha, SIMD_MODE mode)
{
    if (mode == SIMD_NO)
    {
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                argb[i][j][0] = alpha;
                argb[i][j][1] = 1. * y[i][j] - 0.001 * (u[i/2][j/2]-128) + 1.402 * (v[i/2][j/2]-128);
                argb[i][j][2] = 1. * y[i][j] - 0.344 * (u[i/2][j/2]-128) - 0.714 * (v[i/2][j/2]-128);
                argb[i][j][3] = 1. * y[i][j] + 1.772 * (u[i/2][j/2]-128) + 0.001 * (v[i/2][j/2]-128);
            }
    }
    else if (mode == SIMD_MMX)
    {
        MMX_PACK pack[3][3], param[3][3];
        int tmp01 = -0.001 * 64, tmp11 = -0.344 * 64, tmp21 = 1.772 * 64;
        int tmp02 = 1.402 * 64, tmp12 = -0.714 * 64, tmp22 = 0.001 * 64;
        for (int k = 0; k < 4; k++)
        {
            param[0][0].i16[k] = param[1][0].i16[k] = param[2][0].i16[k] = 64;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
        }
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 4)
            {
                for (int k = 0; k < 4; k++)
                {
                    int j_ = j + k;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = y[i][j_];
                    pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = u[i/2][j_/2] - 128;
                    pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = v[i/2][j_/2] - 128;
                }
                for (int k = 0; k < 3; k++)
                {
                    pack[k][0].p = _mm_mullo_pi16(pack[k][0].p, param[k][0].p);
                    pack[k][1].p = _mm_mullo_pi16(pack[k][1].p, param[k][1].p);
                    pack[k][2].p = _mm_mullo_pi16(pack[k][2].p, param[k][2].p);
                }
                pack[0][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[0][0].p, _mm_adds_pi16(pack[0][1].p, pack[0][2].p)), 6);
                pack[1][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[1][0].p, _mm_adds_pi16(pack[1][1].p, pack[1][2].p)), 6);
                pack[2][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[2][0].p, _mm_adds_pi16(pack[2][1].p, pack[2][2].p)), 6);
                for (int k = 0; k < 4; k++)
                {
                    int j_ = j + k;
                    argb[i][j_][0] = alpha;
                    argb[i][j_][1] = pack[0][0].i16[k];
                    argb[i][j_][2] = pack[1][0].i16[k];
                    argb[i][j_][3] = pack[2][0].i16[k];
                }
            }
    }
    else if (mode == SIMD_SSE)
    {
        SSE_PACK pack[3][3], param[3][3];
        int tmp01 = -0.001 * 64, tmp11 = -0.344 * 64, tmp21 = 1.772 * 64;
        int tmp02 = 1.402 * 64, tmp12 = -0.714 * 64, tmp22 = 0.001 * 64;
        for (int k = 0; k < 8; k++)
        {
            param[0][0].i16[k] = param[1][0].i16[k] = param[2][0].i16[k] = 64;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
        }
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int j_ = j + k;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = y[i][j_];
                    pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = u[i/2][j_/2] - 128;
                    pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = v[i/2][j_/2] - 128;
                }
                for (int k = 0; k < 3; k++)
                {
                    pack[k][0].pi = _mm_mullo_epi16(pack[k][0].pi, param[k][0].pi);
                    pack[k][1].pi = _mm_mullo_epi16(pack[k][1].pi, param[k][1].pi);
                    pack[k][2].pi = _mm_mullo_epi16(pack[k][2].pi, param[k][2].pi);
                }
                pack[0][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[0][0].pi, _mm_adds_epi16(pack[0][1].pi, pack[0][2].pi)), 6);
                pack[1][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[1][0].pi, _mm_adds_epi16(pack[1][1].pi, pack[1][2].pi)), 6);
                pack[2][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[2][0].pi, _mm_adds_epi16(pack[2][1].pi, pack[2][2].pi)), 6);
                for (int k = 0; k < 8; k++)
                {
                    int j_ = j + k;
                    argb[i][j_][0] = alpha;
                    argb[i][j_][1] = pack[0][0].i16[k];
                    argb[i][j_][2] = pack[1][0].i16[k];
                    argb[i][j_][3] = pack[2][0].i16[k];
                }
            }
    }
    else if (mode == SIMD_AVX)
    {
        AVX_PACK pack[3][3], param[3][3];
        int tmp01 = -0.001 * 64, tmp11 = -0.344 * 64, tmp21 = 1.772 * 64;
        int tmp02 = 1.402 * 64, tmp12 = -0.714 * 64, tmp22 = 0.001 * 64;
        for (int k = 0; k < 16; k++)
        {
            param[0][0].i16[k] = param[1][0].i16[k] = param[2][0].i16[k] = 64;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
        }
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 16)
            {
                for (int k = 0; k < 16; k++)
                {
                    int j_ = j + k;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = y[i][j_];
                    pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = u[i/2][j_/2] - 128;
                    pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = v[i/2][j_/2] - 128;
                }
                for (int k = 0; k < 3; k++)
                {
                    pack[k][0].pi = _mm256_mullo_epi16(pack[k][0].pi, param[k][0].pi);
                    pack[k][1].pi = _mm256_mullo_epi16(pack[k][1].pi, param[k][1].pi);
                    pack[k][2].pi = _mm256_mullo_epi16(pack[k][2].pi, param[k][2].pi);
                }
                pack[0][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[0][0].pi, _mm256_adds_epi16(pack[0][1].pi, pack[0][2].pi)), 6);
                pack[1][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[1][0].pi, _mm256_adds_epi16(pack[1][1].pi, pack[1][2].pi)), 6);
                pack[2][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[2][0].pi, _mm256_adds_epi16(pack[2][1].pi, pack[2][2].pi)), 6);
                for (int k = 0; k < 16; k++)
                {
                    int j_ = j + k;
                    argb[i][j_][0] = alpha;
                    argb[i][j_][1] = pack[0][0].i16[k];
                    argb[i][j_][2] = pack[1][0].i16[k];
                    argb[i][j_][3] = pack[2][0].i16[k];
                }
            }
    }

    return 0;
}

int argb8888_2_rgb888(SIMD_MODE mode)
{
    if (mode == SIMD_NO)
    {
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                rgb[i][j][0] = argb[i][j][0] / 255. * argb[i][j][1];
                rgb[i][j][1] = argb[i][j][0] / 255. * argb[i][j][2];
                rgb[i][j][2] = argb[i][j][0] / 255. * argb[i][j][3];
            }
    }
    else if (mode == SIMD_MMX)
    {
        MMX_PACK pack[4];
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 4)
            {
                for (int k = 0; k < 4; k++)
                {
                    int j_ = k + j;
                    pack[0].i16[k] = argb[i][j_][0];
                    pack[1].i16[k] = argb[i][j_][1];
                    pack[2].i16[k] = argb[i][j_][2];
                    pack[3].i16[k] = argb[i][j_][3];
                }
                pack[1].p = _mm_srli_pi16(_mm_mullo_pi16(pack[0].p, pack[1].p), 8);
                pack[2].p = _mm_srli_pi16(_mm_mullo_pi16(pack[0].p, pack[2].p), 8);
                pack[3].p = _mm_srli_pi16(_mm_mullo_pi16(pack[0].p, pack[3].p), 8);
                for (int k = 0; k < 4; k++)
                {
                    int j_ = k + j;
                    rgb[i][j_][0] = pack[1].i16[k];
                    rgb[i][j_][1] = pack[2].i16[k];
                    rgb[i][j_][2] = pack[3].i16[k];
                }
            }
    }
    else if (mode == SIMD_SSE)
    {
        SSE_PACK pack[4];
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int j_ = k + j;
                    pack[0].i16[k] = argb[i][j_][0];
                    pack[1].i16[k] = argb[i][j_][1];
                    pack[2].i16[k] = argb[i][j_][2];
                    pack[3].i16[k] = argb[i][j_][3];
                }
                pack[1].pi = _mm_srli_epi16(_mm_mullo_epi16(pack[0].pi, pack[1].pi), 8);
                pack[2].pi = _mm_srli_epi16(_mm_mullo_epi16(pack[0].pi, pack[2].pi), 8);
                pack[3].pi = _mm_srli_epi16(_mm_mullo_epi16(pack[0].pi, pack[3].pi), 8);
                for (int k = 0; k < 8; k++)
                {
                    int j_ = k + j;
                    rgb[i][j_][0] = pack[1].i16[k];
                    rgb[i][j_][1] = pack[2].i16[k];
                    rgb[i][j_][2] = pack[3].i16[k];
                }
            }
    }
    else if (mode == SIMD_AVX)
    {
        AVX_PACK pack[4];
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 16)
            {
                for (int k = 0; k < 16; k++)
                {
                    int j_ = k + j;
                    pack[0].i16[k] = argb[i][j_][0];
                    pack[1].i16[k] = argb[i][j_][1];
                    pack[2].i16[k] = argb[i][j_][2];
                    pack[3].i16[k] = argb[i][j_][3];
                }
                pack[1].pi = _mm256_srli_epi16(_mm256_mullo_epi16(pack[0].pi, pack[1].pi), 8);
                pack[2].pi = _mm256_srli_epi16(_mm256_mullo_epi16(pack[0].pi, pack[2].pi), 8);
                pack[3].pi = _mm256_srli_epi16(_mm256_mullo_epi16(pack[0].pi, pack[3].pi), 8);
                for (int k = 0; k < 16; k++)
                {
                    int j_ = k + j;
                    rgb[i][j_][0] = pack[1].i16[k];
                    rgb[i][j_][1] = pack[2].i16[k];
                    rgb[i][j_][2] = pack[3].i16[k];
                }
            }
    }

    return 0;
}

int store_rgb888(char *a_name)
{
    FILE *file = fopen(a_name, "w");
    if (file == NULL)
        return -1;

    fwrite(rgb, 1, w*h*3, file);

    fclose(file);
    return 0;
}

int rgb888_2_yuv420(int frame, SIMD_MODE mode)
{
    memset(_y[frame], 0, sizeof(unsigned char)*h*w);
    memset(_u[frame], 0, sizeof(unsigned char)*h/2*w/2);
    memset(_v[frame], 0, sizeof(unsigned char)*h/2*w/2);

    if (mode == SIMD_NO)
    {
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                _y[frame][i][j] = 0.299 * rgb[i][j][0] + 0.587 * rgb[i][j][1] + 0.114 * rgb[i][j][2] + 0;
                _u[frame][i/2][j/2] += round((-0.169 * rgb[i][j][0] - 0.331 * rgb[i][j][1] + 0.500 * rgb[i][j][2] + 128) / 4);
                _v[frame][i/2][j/2] += round((0.500 * rgb[i][j][0] - 0.419 * rgb[i][j][1] + 0.081 * rgb[i][j][2] + 128) / 4);
            }
    }
    else if (mode == SIMD_MMX)
    {
        MMX_PACK pack[3][3], param[3][3], add_128;
        int tmp00 = 0.299 * 64, tmp10 = -0.169 * 64, tmp20 = 0.500 * 64;
        int tmp01 = 0.587 * 64, tmp11 = -0.331 * 64, tmp21 = -0.419 * 64;
        int tmp02 = 0.114 * 64, tmp12 = 0.500 * 64, tmp22 = 0.081 * 64;
        int x128 = 128 * 64;
        for (int k = 0; k < 4; k++)
        {
            param[0][0].i16[k] = tmp00;
            param[1][0].i16[k] = tmp10;
            param[2][0].i16[k] = tmp20;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
            add_128.i16[k] = x128;
        }
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 4)
            {
                for (int k = 0; k < 4; k++)
                {
                    int j_ = j + k;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = rgb[i][j_][0];
                    pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = rgb[i][j_][1];
                    pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = rgb[i][j_][2];
                }
                for (int k = 0; k < 3; k++)
                {
                    pack[k][0].p = _mm_mullo_pi16(pack[k][0].p, param[k][0].p);
                    pack[k][1].p = _mm_mullo_pi16(pack[k][1].p, param[k][1].p);
                    pack[k][2].p = _mm_mullo_pi16(pack[k][2].p, param[k][2].p);
                }
                pack[0][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[0][0].p, _mm_adds_pi16(pack[0][1].p, pack[0][2].p)), 6);
                pack[1][0].p = _mm_srli_pi16(_mm_adds_pi16(_mm_adds_pi16(pack[1][0].p, _mm_adds_pi16(pack[1][1].p, pack[1][2].p)), add_128.p), 8);
                pack[2][0].p = _mm_srli_pi16(_mm_adds_pi16(_mm_adds_pi16(pack[2][0].p, _mm_adds_pi16(pack[2][1].p, pack[2][2].p)), add_128.p), 8);
                for (int k = 0; k < 4; k++)
                {
                    int j_ = j + k;
                    _y[frame][i][j_] = pack[0][0].i16[k];
                    _u[frame][i/2][j_/2] += pack[1][0].i16[k];
                    _v[frame][i/2][j_/2] += pack[2][0].i16[k];
                }
            }
    }
    else if (mode == SIMD_SSE)
    {
        SSE_PACK pack[3][3], param[3][3], add_128;
        int tmp00 = 0.299 * 64, tmp10 = -0.169 * 64, tmp20 = 0.500 * 64;
        int tmp01 = 0.587 * 64, tmp11 = -0.331 * 64, tmp21 = -0.419 * 64;
        int tmp02 = 0.114 * 64, tmp12 = 0.500 * 64, tmp22 = 0.081 * 64;
        int x128 = 128 * 64;
        for (int k = 0; k < 8; k++)
        {
            param[0][0].i16[k] = tmp00;
            param[1][0].i16[k] = tmp10;
            param[2][0].i16[k] = tmp20;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
            add_128.i16[k] = x128;
        }
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int j_ = j + k;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = rgb[i][j_][0];
                    pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = rgb[i][j_][1];
                    pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = rgb[i][j_][2];
                }
                for (int k = 0; k < 3; k++)
                {
                    pack[k][0].pi = _mm_mullo_epi16(pack[k][0].pi, param[k][0].pi);
                    pack[k][1].pi = _mm_mullo_epi16(pack[k][1].pi, param[k][1].pi);
                    pack[k][2].pi = _mm_mullo_epi16(pack[k][2].pi, param[k][2].pi);
                }
                pack[0][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[0][0].pi, _mm_adds_epi16(pack[0][1].pi, pack[0][2].pi)), 6);
                pack[1][0].pi = _mm_srli_epi16(_mm_adds_epi16(_mm_adds_epi16(pack[1][0].pi, _mm_adds_epi16(pack[1][1].pi, pack[1][2].pi)), add_128.pi), 8);
                pack[2][0].pi = _mm_srli_epi16(_mm_adds_epi16(_mm_adds_epi16(pack[2][0].pi, _mm_adds_epi16(pack[2][1].pi, pack[2][2].pi)), add_128.pi), 8);
                for (int k = 0; k < 8; k++)
                {
                    int j_ = j + k;
                    _y[frame][i][j_] = pack[0][0].i16[k];
                    _u[frame][i/2][j_/2] += pack[1][0].i16[k];
                    _v[frame][i/2][j_/2] += pack[2][0].i16[k];
                }
            }
    }
    else if (mode == SIMD_AVX)
    {
        AVX_PACK pack[3][3], param[3][3], add_128;
        int tmp00 = 0.299 * 64, tmp10 = -0.169 * 64, tmp20 = 0.500 * 64;
        int tmp01 = 0.587 * 64, tmp11 = -0.331 * 64, tmp21 = -0.419 * 64;
        int tmp02 = 0.114 * 64, tmp12 = 0.500 * 64, tmp22 = 0.081 * 64;
        int x128 = 128 * 64;
        for (int k = 0; k < 16; k++)
        {
            param[0][0].i16[k] = tmp00;
            param[1][0].i16[k] = tmp10;
            param[2][0].i16[k] = tmp20;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
            add_128.i16[k] = x128;
        }
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 16)
            {
                for (int k = 0; k < 16; k++)
                {
                    int j_ = j + k;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = rgb[i][j_][0];
                    pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = rgb[i][j_][1];
                    pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = rgb[i][j_][2];
                }
                for (int k = 0; k < 3; k++)
                {
                    pack[k][0].pi = _mm256_mullo_epi16(pack[k][0].pi, param[k][0].pi);
                    pack[k][1].pi = _mm256_mullo_epi16(pack[k][1].pi, param[k][1].pi);
                    pack[k][2].pi = _mm256_mullo_epi16(pack[k][2].pi, param[k][2].pi);
                }
                pack[0][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[0][0].pi, _mm256_adds_epi16(pack[0][1].pi, pack[0][2].pi)), 6);
                pack[1][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(_mm256_adds_epi16(pack[1][0].pi, _mm256_adds_epi16(pack[1][1].pi, pack[1][2].pi)), add_128.pi), 8);
                pack[2][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(_mm256_adds_epi16(pack[2][0].pi, _mm256_adds_epi16(pack[2][1].pi, pack[2][2].pi)), add_128.pi), 8);
                for (int k = 0; k < 16; k++)
                {
                    int j_ = j + k;
                    _y[frame][i][j_] = pack[0][0].i16[k];
                    _u[frame][i/2][j_/2] += pack[1][0].i16[k];
                    _v[frame][i/2][j_/2] += pack[2][0].i16[k];
                }
            }
    }

    return 0;
}

int store_yuv420_vedio(char *a_name)
{
    FILE *file = fopen(a_name, "w");
    if (file == NULL)
        return -1;

    for (int i = 0; i < n_frame; i++)
    {
        fwrite(_y[i], 1, w*h, file);
        fwrite(_u[i], 1, w/2*h/2, file);
        fwrite(_v[i], 1, w/2*h/2, file);
    }

    fclose(file);
    return 0;
}

int store_yuv420_frame(char *a_name, int frame)
{
    FILE *file = fopen(a_name, "w");
    if (file == NULL)
        return -1;

    fwrite(_y[frame], 1, w*h, file);
    fwrite(_u[frame], 1, w/2*h/2, file);
    fwrite(_v[frame], 1, w/2*h/2, file);

    fclose(file);
    return 0;
}

int load_yuv420_2(char *a1_name, char *a2_name)
{
    FILE *file = NULL;
    if ((file = fopen(a1_name, "r")) == NULL)
        return -1;

    fseek(file, 0, 0);
    fread(y2[0], 1, w*h, file);
    fread(u2[0], 1, w/2*h/2, file);
    fread(v2[0], 1, w/2*h/2, file);

    fclose(file);

    if ((file = fopen(a2_name, "r")) == NULL)
        return -1;

    fseek(file, 0, 0);
    fread(y2[1], 1, w*h, file);
    fread(u2[1], 1, w/2*h/2, file);
    fread(v2[1], 1, w/2*h/2, file);

    fclose(file);

    return 0;
}

int yuv420_2_rgb888_2(SIMD_MODE mode)
{
    if (mode == SIMD_NO)
    {
        for (int f = 0; f < 2; f++)
            for (int i = 0; i < h; i++)
                for (int j = 0; j < w; j++)
                {
                    rgb2[f][i][j][0] = y2[f][i][j] - 0.001 * (u2[f][i/2][j/2]-128) + 1.402 * (v2[f][i/2][j/2]-128);
                    rgb2[f][i][j][1] = y2[f][i][j] - 0.344 * (u2[f][i/2][j/2]-128) - 0.714 * (v2[f][i/2][j/2]-128);
                    rgb2[f][i][j][2] = y2[f][i][j] + 1.772 * (u2[f][i/2][j/2]-128) + 0.001 * (v2[f][i/2][j/2]-128);
                }
    }
    else if (mode == SIMD_MMX)
    {
        MMX_PACK pack[3][3], param[3][3];
        int tmp01 = -0.001 * 64, tmp11 = -0.344 * 64, tmp21 = 1.772 * 64;
        int tmp02 = 1.402 * 64, tmp12 = -0.714 * 64, tmp22 = 0.001 * 64;
        for (int k = 0; k < 4; k++)
        {
            param[0][0].i16[k] = param[1][0].i16[k] = param[2][0].i16[k] = 64;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
        }
        for (int f = 0; f < 2; f++)
            for (int i = 0; i < h; i++)
                for (int j = 0; j < w; j += 4)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        int j_ = j + k;
                        pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = y2[f][i][j_];
                        pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = u2[f][i/2][j_/2] - 128;
                        pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = v2[f][i/2][j_/2] - 128;
                    }
                    for (int k = 0; k < 3; k++)
                    {
                        pack[k][0].p = _mm_mullo_pi16(pack[k][0].p, param[k][0].p);
                        pack[k][1].p = _mm_mullo_pi16(pack[k][1].p, param[k][1].p);
                        pack[k][2].p = _mm_mullo_pi16(pack[k][2].p, param[k][2].p);
                    }
                    pack[0][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[0][0].p, _mm_adds_pi16(pack[0][1].p, pack[0][2].p)), 6);
                    pack[1][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[1][0].p, _mm_adds_pi16(pack[1][1].p, pack[1][2].p)), 6);
                    pack[2][0].p = _mm_srli_pi16(_mm_adds_pi16(pack[2][0].p, _mm_adds_pi16(pack[2][1].p, pack[2][2].p)), 6);
                    for (int k = 0; k < 4; k++)
                    {
                        int j_ = j + k;
                        rgb2[f][i][j_][0] = pack[0][0].i16[k];
                        rgb2[f][i][j_][1] = pack[1][0].i16[k];
                        rgb2[f][i][j_][2] = pack[2][0].i16[k];
                    }
                }
    }
    else if (mode == SIMD_SSE)
    {
        SSE_PACK pack[3][3], param[3][3];
        int tmp01 = -0.001 * 64, tmp11 = -0.344 * 64, tmp21 = 1.772 * 64;
        int tmp02 = 1.402 * 64, tmp12 = -0.714 * 64, tmp22 = 0.001 * 64;
        for (int k = 0; k < 8; k++)
        {
            param[0][0].i16[k] = param[1][0].i16[k] = param[2][0].i16[k] = 64;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
        }
        for (int f = 0; f < 2; f++)
            for (int i = 0; i < h; i++)
                for (int j = 0; j < w; j += 8)
                {
                    for (int k = 0; k < 8; k++)
                    {
                        int j_ = j + k;
                        pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = y2[f][i][j_];
                        pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = u2[f][i/2][j_/2] - 128;
                        pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = v2[f][i/2][j_/2] - 128;
                    }
                    for (int k = 0; k < 3; k++)
                    {
                        pack[k][0].pi = _mm_mullo_epi16(pack[k][0].pi, param[k][0].pi);
                        pack[k][1].pi = _mm_mullo_epi16(pack[k][1].pi, param[k][1].pi);
                        pack[k][2].pi = _mm_mullo_epi16(pack[k][2].pi, param[k][2].pi);
                    }
                    pack[0][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[0][0].pi, _mm_adds_epi16(pack[0][1].pi, pack[0][2].pi)), 6);
                    pack[1][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[1][0].pi, _mm_adds_epi16(pack[1][1].pi, pack[1][2].pi)), 6);
                    pack[2][0].pi = _mm_srli_epi16(_mm_adds_epi16(pack[2][0].pi, _mm_adds_epi16(pack[2][1].pi, pack[2][2].pi)), 6);
                    for (int k = 0; k < 8; k++)
                    {
                        int j_ = j + k;
                        rgb2[f][i][j_][0] = pack[0][0].i16[k];
                        rgb2[f][i][j_][1] = pack[1][0].i16[k];
                        rgb2[f][i][j_][2] = pack[2][0].i16[k];
                    }
                }
    }
    else if (mode == SIMD_AVX)
    {
        AVX_PACK pack[3][3], param[3][3];
        int tmp01 = -0.001 * 64, tmp11 = -0.344 * 64, tmp21 = 1.772 * 64;
        int tmp02 = 1.402 * 64, tmp12 = -0.714 * 64, tmp22 = 0.001 * 64;
        for (int k = 0; k < 16; k++)
        {
            param[0][0].i16[k] = param[1][0].i16[k] = param[2][0].i16[k] = 64;
            param[0][1].i16[k] = tmp01;
            param[1][1].i16[k] = tmp11;
            param[2][1].i16[k] = tmp21;
            param[0][2].i16[k] = tmp02;
            param[1][2].i16[k] = tmp12;
            param[2][2].i16[k] = tmp22;
        }
        for (int f = 0; f < 2; f++)
            for (int i = 0; i < h; i++)
                for (int j = 0; j < w; j += 16)
                {
                    for (int k = 0; k < 16; k++)
                    {
                        int j_ = j + k;
                        pack[0][0].i16[k] = pack[1][0].i16[k] = pack[2][0].i16[k] = y2[f][i][j_];
                        pack[0][1].i16[k] = pack[1][1].i16[k] = pack[2][1].i16[k] = u2[f][i/2][j_/2] - 128;
                        pack[0][2].i16[k] = pack[1][2].i16[k] = pack[2][2].i16[k] = v2[f][i/2][j_/2] - 128;
                    }
                    for (int k = 0; k < 3; k++)
                    {
                        pack[k][0].pi = _mm256_mullo_epi16(pack[k][0].pi, param[k][0].pi);
                        pack[k][1].pi = _mm256_mullo_epi16(pack[k][1].pi, param[k][1].pi);
                        pack[k][2].pi = _mm256_mullo_epi16(pack[k][2].pi, param[k][2].pi);
                    }
                    pack[0][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[0][0].pi, _mm256_adds_epi16(pack[0][1].pi, pack[0][2].pi)), 6);
                    pack[1][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[1][0].pi, _mm256_adds_epi16(pack[1][1].pi, pack[1][2].pi)), 6);
                    pack[2][0].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[2][0].pi, _mm256_adds_epi16(pack[2][1].pi, pack[2][2].pi)), 6);
                    for (int k = 0; k < 16; k++)
                    {
                        int j_ = j + k;
                        rgb2[f][i][j_][0] = pack[0][0].i16[k];
                        rgb2[f][i][j_][1] = pack[1][0].i16[k];
                        rgb2[f][i][j_][2] = pack[2][0].i16[k];
                    }
                }
    }

    return 0;
}

int merge_rgb888(int alpha, SIMD_MODE mode)
{
    if (mode == SIMD_NO)
    {
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                rgb[i][j][0] = (alpha * rgb2[0][i][j][0] + (255 - alpha) * rgb2[1][i][j][0]) / 255.;
                rgb[i][j][1] = (alpha * rgb2[0][i][j][1] + (255 - alpha) * rgb2[1][i][j][1]) / 255.;
                rgb[i][j][2] = (alpha * rgb2[0][i][j][2] + (255 - alpha) * rgb2[1][i][j][2]) / 255.;
            }
    }
    else if (mode == SIMD_MMX)
    {
        MMX_PACK pack[2][4], sub_256;
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 4)
            {
                for (int k = 0; k < 4; k++)
                {
                    int j_ = k + j;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = alpha;
                    pack[0][1].i16[k] = rgb2[0][i][j_][0];
                    pack[1][1].i16[k] = rgb2[1][i][j_][0];
                    pack[0][2].i16[k] = rgb2[0][i][j_][1];
                    pack[1][2].i16[k] = rgb2[1][i][j_][1];
                    pack[0][3].i16[k] = rgb2[0][i][j_][2];
                    pack[1][3].i16[k] = rgb2[1][i][j_][2];
                    sub_256.i16[k] = 256;
                }
                pack[1][0].p = _mm_subs_pi16(sub_256.p, pack[1][0].p);
                for (int x = 0; x < 2; x++)
                    for (int y = 1; y <= 3; y++)
                        pack[x][y].p = _mm_mullo_pi16(pack[x][0].p, pack[x][y].p);
                pack[0][1].p = _mm_srli_pi16(_mm_adds_pi16(pack[0][1].p, pack[1][1].p), 8);
                pack[0][2].p = _mm_srli_pi16(_mm_adds_pi16(pack[0][2].p, pack[1][2].p), 8);
                pack[0][3].p = _mm_srli_pi16(_mm_adds_pi16(pack[0][3].p, pack[1][3].p), 8);
                for (int k = 0; k < 4; k++)
                {
                    int j_ = k + j;
                    rgb[i][j_][0] = pack[0][1].i16[k];
                    rgb[i][j_][1] = pack[0][2].i16[k];
                    rgb[i][j_][2] = pack[0][3].i16[k];
                }
            }
    }
    else if (mode == SIMD_SSE)
    {
        SSE_PACK pack[2][4], sub_256;
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int j_ = k + j;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = alpha;
                    pack[0][1].i16[k] = rgb2[0][i][j_][0];
                    pack[1][1].i16[k] = rgb2[1][i][j_][0];
                    pack[0][2].i16[k] = rgb2[0][i][j_][1];
                    pack[1][2].i16[k] = rgb2[1][i][j_][1];
                    pack[0][3].i16[k] = rgb2[0][i][j_][2];
                    pack[1][3].i16[k] = rgb2[1][i][j_][2];
                    sub_256.i16[k] = 256;
                }
                pack[1][0].pi = _mm_subs_epi16(sub_256.pi, pack[1][0].pi);
                for (int x = 0; x < 2; x++)
                    for (int y = 1; y <= 3; y++)
                        pack[x][y].pi = _mm_mullo_epi16(pack[x][0].pi, pack[x][y].pi);
                pack[0][1].pi = _mm_srli_epi16(_mm_adds_epi16(pack[0][1].pi, pack[1][1].pi), 8);
                pack[0][2].pi = _mm_srli_epi16(_mm_adds_epi16(pack[0][2].pi, pack[1][2].pi), 8);
                pack[0][3].pi = _mm_srli_epi16(_mm_adds_epi16(pack[0][3].pi, pack[1][3].pi), 8);
                for (int k = 0; k < 8; k++)
                {
                    int j_ = k + j;
                    rgb[i][j_][0] = pack[0][1].i16[k];
                    rgb[i][j_][1] = pack[0][2].i16[k];
                    rgb[i][j_][2] = pack[0][3].i16[k];
                }
            }
    }
    else if (mode == SIMD_AVX)
    {
        AVX_PACK pack[2][4], sub_256;
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j += 16)
            {
                for (int k = 0; k < 16; k++)
                {
                    int j_ = k + j;
                    pack[0][0].i16[k] = pack[1][0].i16[k] = alpha;
                    pack[0][1].i16[k] = rgb2[0][i][j_][0];
                    pack[1][1].i16[k] = rgb2[1][i][j_][0];
                    pack[0][2].i16[k] = rgb2[0][i][j_][1];
                    pack[1][2].i16[k] = rgb2[1][i][j_][1];
                    pack[0][3].i16[k] = rgb2[0][i][j_][2];
                    pack[1][3].i16[k] = rgb2[1][i][j_][2];
                    sub_256.i16[k] = 256;
                }
                pack[1][0].pi = _mm256_subs_epi16(sub_256.pi, pack[1][0].pi);
                for (int x = 0; x < 2; x++)
                    for (int y = 1; y <= 3; y++)
                        pack[x][y].pi = _mm256_mullo_epi16(pack[x][0].pi, pack[x][y].pi);
                pack[0][1].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[0][1].pi, pack[1][1].pi), 8);
                pack[0][2].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[0][2].pi, pack[1][2].pi), 8);
                pack[0][3].pi = _mm256_srli_epi16(_mm256_adds_epi16(pack[0][3].pi, pack[1][3].pi), 8);
                for (int k = 0; k < 16; k++)
                {
                    int j_ = k + j;
                    rgb[i][j_][0] = pack[0][1].i16[k];
                    rgb[i][j_][1] = pack[0][2].i16[k];
                    rgb[i][j_][2] = pack[0][3].i16[k];
                }
            }
    }

    return 0;
}
