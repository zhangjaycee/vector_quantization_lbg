#define CODE_K 8
#define BOOK_SIZE 256
#define WORD_WIDTH 2
#define WORD_SIZE 4
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>

typedef unsigned char code_t;
typedef unsigned char index_t;
int main()
{
    printf("==== 数据压缩大作业3 - 基于LBG的图像矢量量化 张佳辰13020188003 压缩程序 ====\n\n");
    int i,j,size0;
    int rows, cols, parts;
    int width, height;
    FILE *fp_in, *fp_encode;
    code_t codebook[BOOK_SIZE][WORD_SIZE];
    //read origin data
    fp_in = fopen("lena512x512.raw","rb");
    fp_encode = fopen("compress.raw","wb");
    if(!fp_in || !fp_encode){
        printf("打开文件失败， 请确定当前目录有lena512x512.raw文件再试\n");
        return -1;
    }else{
        fseek(fp_in, 0, SEEK_END);
        size0 = ftell(fp_in);
        fseek(fp_in, 0, SEEK_SET);
    }
    if(size0 % WORD_SIZE != 0){
        printf("error\n");
        return -1;
    }
    width = height = sqrt(size0);
    rows = cols = width/WORD_WIDTH;
    parts = size0 / WORD_SIZE;
    //printf("rows = cols = %d\n",rows);
    code_t *data=(code_t *)malloc(size0 * sizeof(code_t));//rows*cols
    fread(data,sizeof(code_t),size0,fp_in);
    //cut the data by WORD_SIZE
    code_t *parted_data=(code_t *)malloc(size0 * sizeof(code_t));
    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            int tmp_i = i/WORD_WIDTH * cols + j/WORD_WIDTH;
            int tmp_j = i%WORD_WIDTH*WORD_WIDTH + j%WORD_WIDTH;
            parted_data[tmp_i*WORD_SIZE + tmp_j] = data[i*width + j];
            if(i==0 && j==0){
            }
        }
    }
    //init codebook
    srand((unsigned int) time(NULL)); 
    for(i = 0; i < BOOK_SIZE; i++){
        for(j = 0; j < WORD_SIZE; j++){
            int rand_i = rand() % parts;
            codebook[i][j] = parted_data[rand_i*WORD_SIZE+j];
        }
    }
    //lbg algorithm
    double d0 = 0.0;
    double d1 = 0.0;
    int near_count[BOOK_SIZE];
    index_t near_code[parts];
    for(i = 0; i < parts; i++){
        for(j = 0; j < WORD_SIZE; j++){
            d0 += pow(parted_data[i*WORD_SIZE+j]-codebook[0][j], 2);
        }
    }
    double eps = 0.001;
    int algo_count = 0;
    for(;;){//loop until (d0-d1)/d1 < esp
        algo_count++;
        d1 = 0.0;
        for(i = 0; i < BOOK_SIZE; i++){//init nearest times count array
            near_count[i] = 0;
        }
        for(i = 0; i < parts; i++){
            double min = 0.0;
            near_code[i] = 0;
            for(j = 0; j < WORD_SIZE; j++){//min init by the 1st vector in codebook
                min += pow(parted_data[i*WORD_SIZE+j] - codebook[0][j], 2);
            }
            for(j = 1; j < BOOK_SIZE; j++){//continue traverse other vectors in codebook
                double d = 0.0;
                for(int k = 0; k < WORD_SIZE; k++){
                    d += pow(parted_data[i*WORD_SIZE+k]-codebook[j][k], 2);
                    if(d > min){
                        break;
                    }
                }
                if(d < min){
                    min = d;
                    near_code[i] = j;
                }
            }
            near_count[near_code[i]] += 1;
            d1 += min;
        }
        printf("\r [迭代进度]  第%d次迭代，现在(d0-d1)/d1为%f\t(epsilon = %.3f)",algo_count, (d0 - d1)/d1, eps);
        fflush(stdout);
        if(abs((d0 - d1))/d1 < eps){
            break;
        }
        d0 = d1;
        for(int i = 0; i < BOOK_SIZE; i++){
            if(near_count[i] != 0){
                int tmp_part[WORD_SIZE] = {0};
                for(j = 0; j < parts; j++){
                    if(near_code[j] == i){
                        for(int k = 0; k < WORD_SIZE; k++){
                            tmp_part[k] += parted_data[j*WORD_SIZE+k];
                        }
                    }
                }
                for(j = 0; j < WORD_SIZE; j++){
                    codebook[i][j] = tmp_part[j] / near_count[i];
                }
            } else{
                for(j = 0; j < WORD_SIZE; j++){
                    int rand_i = rand() % parts;
                    codebook[i][j] = parted_data[rand_i*WORD_SIZE+j];
                }
            }
        }
    }
    //save the codebook to encode file
    int write_buffer =  BOOK_SIZE;
    fwrite(&write_buffer, sizeof(int), 1, fp_encode);
    write_buffer = WORD_WIDTH;
    fwrite(&write_buffer, sizeof(int), 1, fp_encode);
    write_buffer =  width;
    fwrite(&write_buffer, sizeof(int), 1, fp_encode);
    for(i = 0; i < BOOK_SIZE; i++){
        for(j = 0; j < WORD_SIZE; j++){
            fwrite(&codebook[i][j], sizeof(code_t), 1, fp_encode);
        }
    }
    //save code indexs to encode file
    for(i = 0; i < parts; i++){
        fwrite(&near_code[i], sizeof(index_t), 1, fp_encode);
    }

    free(data);
    free(parted_data);
    fseek(fp_encode, 0, SEEK_END);
    int size1 = ftell(fp_encode);
    fclose(fp_in);
    fclose(fp_encode); 
    float ratio = (float)size1/size0*100;
    int bksize = BOOK_SIZE;
    int wdwid = WORD_WIDTH;
    printf("\n [压缩完成]  图片大小为%dx%d, 码本大小为%d，码字大小为%d\n", width, height, bksize, wdwid);
    printf("             原始文件大小为%.2f kB ,压缩后文件为%.2f kB , 压缩率为%.2f%%\n", 
            (float)size0/1000, (float)size1/1000, ratio);
    printf("\n============================================================================\n");
    return 0;
}
   
