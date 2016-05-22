#include<stdio.h>
typedef unsigned char code_t;
typedef unsigned char index_t;
int main()
{
    printf("==== 数据压缩大作业3 - 基于LBG的图像矢量量化 张佳辰13020188003 解压程序 ====\n\n");
    FILE *fp_encode;
    FILE *fp_decode;
    int i, j, k;
    int BOOK_SIZE, WORD_WIDTH, WORD_SIZE, width, cols, parts;
    fp_encode = fopen("compress.raw", "rb");
    fp_decode = fopen("restore.raw", "wb");
    
    fread(&BOOK_SIZE, sizeof(int), 1, fp_encode);
    fread(&WORD_WIDTH, sizeof(int), 1, fp_encode);
    fread(&width, sizeof(int), 1, fp_encode);
    WORD_SIZE = WORD_WIDTH*WORD_WIDTH;
    cols = width / WORD_WIDTH;
    parts = cols * cols;
    code_t codebook[BOOK_SIZE][WORD_SIZE];
    for(i = 0; i < BOOK_SIZE; i++){
        for(j = 0; j < WORD_SIZE; j++){
            fread(&codebook[i][j], sizeof(code_t), 1, fp_encode);
        }
    }
    int restruct[width][width];
    index_t index;
    int re_i = 0, re_j = 0;
    for(i = 0; i < parts; i++){
        fread(&index, sizeof(index_t), 1, fp_encode);
        restruct[re_i][re_j] = codebook[index][0];
        restruct[re_i][re_j+1] = codebook[index][1];
        restruct[re_i+1][re_j] = codebook[index][2];
        restruct[re_i+1][re_j+1] = codebook[index][3];
        if(re_j + 2 < width){
            re_j += 2;
        }else{
            re_j = 0;
            re_i += 2;
        }

    }
    for(i = 0; i < width; i++){
        for(j = 0; j < width; j++){
            fwrite(&restruct[i][j], 1, 1, fp_decode);
        }
    }
    fclose(fp_encode);
    fclose(fp_decode);
    printf(" [解压完成]  图片大小为%dx%d, 码本大小为%d，码字大小为%d\n\n", width, width, BOOK_SIZE, WORD_WIDTH);
    printf("============================================================================\n");
    return 0;

}
