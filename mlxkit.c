/*****************************************************************************
 * MLXkit: Encodes C64 .PRG files into MLX binary listings
 * Copyright 2026 Michael C. Martin
 * Released under the zlib license.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MLX versions 2 and 3 store the program in the $3200-$9FFF region */
#define MAX_SIZE 0x6e00

static unsigned char prgbuffer[MAX_SIZE+1];
static unsigned int prgstart, prgsize;

static int load_prg(const char *fname)
{
    FILE *f = fopen(fname, "rb");
    int i, c;
    if (!f) {
        fprintf(stderr, "%s: Could not open file\n", fname);
        return 0;
    }
    /* Clear the program buffers */
    for (i = 0; i < MAX_SIZE+1; ++i)
        prgbuffer[i] = 0;
    prgstart = 0;
    prgsize = 0;
    /* Load start address */
    c = fgetc(f);
    if (c < 0) {
        fprintf(stderr, "%s: Not a PRG file\n", fname);
        fclose(f);
        return 0;
    }
    prgstart = (unsigned int)c;
    c = fgetc(f);
    if (c < 0) {
        fprintf(stderr, "%s: Not a PRG file\n", fname);
        prgstart = 0;
        fclose(f);
        return 0;
    }
    prgstart |= (unsigned int)(c << 8);
    /* Load file into buffer to determine size */
    for (i = 0; i < MAX_SIZE+1; ++i) {
        c = fgetc(f);
        if (c < 0) break;
        prgbuffer[i] = (unsigned char)c;
        ++prgsize;
    }
    fclose(f);
    if (prgsize > MAX_SIZE) {
        prgsize = MAX_SIZE;
        fprintf(stderr, "%s: Warning: File is too large, truncating to 27.5KB\n", fname);
    }
    return 1;
}

static void mlx1_encode(void)
{
    unsigned int prgend, pc, ck;
    prgend = prgstart + prgsize - 1;
    if (prgstart >= 0x2000 && prgstart < 0xa000) {
        printf("Before loading MLX, enter these commands:\n");
        printf("\n  POKE 55,255:POKE 56,%u:CLR\n\n",(prgstart >> 8) - 1);
    } else if (prgstart >= 0x800 && prgstart < 0x2000) {
        unsigned int mlx_start_page = (prgend + 255) >> 8;
        printf("Before loading MLX, enter these commands:\n");
        printf("\n  POKE 44,%u:POKE %u,0:NEW\n\n", mlx_start_page, mlx_start_page << 8);
    }

    printf("Start address: %u\n", prgstart);
    printf("End address:   %u\n\n", prgend);

    for (pc = prgstart; pc <= prgend; pc += 6) {
        int i;
        ck = pc;
        printf("%u ",pc);
        for (i = 0; i < 6; ++i) {
            int c = (pc + i > prgend) ? 0 : prgbuffer[pc - prgstart + i];
            ck += c;
            printf("%c%03d", i ? ',' : ':', c);
        }
        printf(",%03u\n", ck & 255);
    }
}

static void mlx2_encode(void)
{
    unsigned int prgend, pc, ck, ck2;
    prgend = prgstart + prgsize - 1;
    printf("Start address: %04X\n", prgstart);
    printf("End address:   %04X\n\n", prgend);

    for (pc = prgstart; pc <= prgend; pc += 8) {
        int i;
        ck = (pc - 254 * (pc >> 8)) % 255; /* Yes, %, not & */
	ck2 = pc;
        printf("%04X",pc);
        for (i = 0; i < 8; ++i) {
            int c = (pc + i > prgend) ? 0 : prgbuffer[pc - prgstart + i];
            ck = ((ck << 1) + c) % 255;
	    ck2 |= c;
            printf("%c%02X", i ? ' ' : ':', c);
        }
	if (ck == 0 && ck2 != 0) ck = 255;
        printf(" %02X\n", ck);
    }
}

static void mlx3_encode(void)
{
    unsigned int prgend, pc, s1, s2, hc, ck;
    prgend = prgstart + prgsize - 1;
    printf("Start address: %04X\n", prgstart);
    printf("End address:   %04X\n\n", prgend);

    for (pc = prgstart; pc <= prgend; pc += 10) {
        int i;
        s1 = pc & 0xff;
        s2 = (pc >> 8) & 0xff;
        hc = 0;
        printf("%04X",pc);
        for (i = 0; i < 10; ++i) {
            int c = (pc + i > prgend) ? 0 : prgbuffer[pc - prgstart + i];
            s1 += c;
            s2 += s1;
            if (c > 127) ++hc;
            printf("%c%02X", i ? ' ' : '-', c);
        }
        ck = (s1 + s2 + hc) & 0xff;
        if (!ck) ck = 255;
        printf(" %02X\n", ck);
    }
}

static int usage(FILE *f)
{
    fprintf(f, "Usage:\n    mlxkit [options] <file.prg>\n");
    fprintf(f, "\nOptions:\n");
    fprintf(f, "    -h, --help      Display this message\n");
    fprintf(f, "    -v, --version   Display version and credits\n");
    fprintf(f, "    -1, -2, -3      Output MLX format 1, 2, or 3\n");
    fprintf(f, "\nMLX format 2 will be used by default if none specified.\n");
    return 1;
}

static void credits(void)
{
    printf("MLXkit: C64 Binary File Encoder\n");
    printf("Version 1.0, (c) 2026 Michael C. Martin.\n");
    printf("Distributed under the zlib license.\n");
}

int main(int argc, char **argv)
{
    const char *fname = NULL;
    int i, version = -1;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(stdout);
            return 0;
        }
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            credits();
            return 0;
        }
        if (!strcmp(argv[i], "-1") || !strcmp(argv[i], "-2") || !strcmp(argv[i], "-3")) {
            if (version > 0) {
                fprintf(stderr, "Error: multiple format request options\n");
                return usage(stderr);
            }
            version = argv[i][1] - '0';
        } else if (fname) {
            fprintf(stderr, "Error: multiple input files are not allowed\n");
            return usage(stderr);
        } else {
            fname = argv[i];
        }
    }
    if (!fname) {
        if (argc > 1) {
            fprintf(stderr, "Error: no input file specified\n");
        }
        return usage(stderr);
    }
    if (!load_prg(fname)) {
        return 1;
    }    
    switch(version) {
        case 1: mlx1_encode(); break;
        case 3: mlx3_encode(); break;
        default: mlx2_encode(); break;
    }
    return 0;
}
