#include <stdio.h>
#include <stdlib.h>
#include "defrag.h"
#define BLOCK_SIZE 512

int rb_num = 0;
int nFileLen = 0;

typedef struct read_block
{
    int block_number;
    int seq_number;
}RB;

typedef struct write_block
{
    char buffer[512];
    int seq_number;
}WB;

void read_iblocks(RB *rb, SB* sb, IN* in , FILE* f, int ib)
{
    fseek(f,0,SEEK_CUR);
    int cur_offset = ftell(f);
    fseek(f,(ib + 6) * (sb -> size), SEEK_SET);
    int k;
    for(k = 0; k < (sb -> size / 4);k ++)
    {
        fread(&rb[rb_num].block_number, 4, 1, f);
        if(rb[rb_num].block_number > (nFileLen / 512) || rb[rb_num].block_number <= 0)
        {
            break;
        }
        printf("iblock no. %d : %d\n",rb_num, rb[rb_num].block_number);
        rb[rb_num].seq_number = rb_num;
        rb_num ++;
    }
    fseek(f,cur_offset,SEEK_SET);
}

void read_i2blocks(RB *rb, SB* sb, IN* in , FILE* f)
{
    int iblocks[(sb -> size / 4)];
    fseek(f,0,SEEK_CUR);
    int cur_offset = ftell(f);
    fseek(f,(in -> i2block + 6) * (sb -> size), SEEK_SET);
    int j;
    for(j = 0; j < (sb -> size / 4); j++)
    {
        fread(&iblocks[j], 4, 1, f);
        if(iblocks[j] > (nFileLen / 512) || iblocks[j] <= 0)
        {
            break;
        }
        read_iblocks(rb, sb, in , f, iblocks[j]);
    }
    fseek(f,cur_offset,SEEK_SET);
}

void read_i3blocks(RB *rb, SB* sb, IN* in , FILE* f)
{
    int iblocks[(sb -> size / 4)];
    fseek(f,0,SEEK_CUR);
    int cur_offset = ftell(f);
    fseek(f,(in -> i2block + 6) * (sb -> size), SEEK_SET);
    int j;
    for(j = 0; j < (sb -> size / 4); j++)
    {
        fread(&iblocks[j], 4, 1, f);
        if(iblocks[j] > (nFileLen / 512) || iblocks[j] <= 0)
        {
            break;
        }
        read_i2blocks(rb, sb, in , f);
    }
    fseek(f,cur_offset,SEEK_SET);
}

void read_content(WB* wb, RB *rb, SB* sb, IN* in , FILE* f)
{
    fseek(f,0,SEEK_CUR);
    int cur_offset = ftell(f);
    fseek(f,(rb[0].block_number + 6) * (sb -> size), SEEK_SET);
    //wb[0].buffer = malloc(sb -> size);
    fread(wb[0].buffer, sb -> size, 1, f);
    int i;
    for(i = 1; i < rb_num; i++)
    {
        fseek(f,(rb[i].block_number - rb[i - 1].block_number - 1) * (sb -> size), SEEK_CUR);
        //wb[i].buffer = malloc(sb -> size);
        fread(wb[i].buffer, sb -> size, 1, f);
    }
    fseek(f,cur_offset,SEEK_SET);
}

void *cmp1(const void * a, const void * b)
{
    return ((RB*)a) -> block_number - ((RB*)b) -> block_number;
}

void *cmp2(const void * a, const void * b)
{
    return ((WB*)a) -> seq_number - ((WB*)b) -> seq_number;
}

void read_and_write_file(SB* sb, IN* in , FILE* f)
{
    int file_size = (in -> size % sb -> size == 0) ? (in -> size / sb -> size) : ((in -> size / sb -> size) + 1);
    printf("file size = %d\n", file_size);
    RB rb[file_size];
    WB wb[file_size];
    int i;
    for(i = 0; i < N_DBLOCKS; i ++)
    {
        if(in -> dblocks[i] == 0) break;
        rb[rb_num].block_number = in -> dblocks[i];
        rb[rb_num].seq_number = rb_num;
        rb_num ++;
    }
    if(file_size > N_DBLOCKS)
    {
        int iblocks_num = (file_size % (sb -> size / 4) == 0) ? (file_size / (sb -> size / 4)) : (file_size / (sb -> size / 4) + 1);
        printf("iblocks_num = %d\n", iblocks_num);
        //int iblocks_offset = file_size - (iblocks_num - 1) * (sb -> size / 4);
        //printf("iblocks_offset = %d\n", iblocks_offset);
        int j;
        for(j = 0; j < iblocks_num;j ++)
        {
            if(in -> iblocks[j] == 0) break;
            read_iblocks(rb, sb, in , f, in -> iblocks[j]);
        }
    }
    if(file_size > N_DBLOCKS + N_IBLOCKS * (sb -> size / 4))
    {
        read_i2blocks(rb, sb, in , f);
    }
    if(file_size > N_DBLOCKS + N_IBLOCKS * (sb -> size / 4) + (sb -> size / 4) * (sb -> size / 4))
    {
        read_i3blocks(rb, sb, in , f);
    }
    qsort(rb, file_size, sizeof(rb[0]), cmp1);
    read_content(wb, rb, sb, in , f);
    for(i = 0; i < rb_num; i ++)
    {
        wb[i].seq_number = rb[i].seq_number;
    }
    printf("asdasdsadasdasdasdasdasdasda = %d\n",wb[1].seq_number);
    qsort(wb, file_size, sizeof(wb[0]), cmp2);
    printf("asdasdsadasdasdasdasdasdasda = %d\n",wb[1].seq_number);
    FILE * f2;
    f2 = fopen("datafile-defrag", "a+");
    for(i = 0; i < rb_num; i++)
    {
        fwrite(wb[i].buffer, sb -> size, 1, f2);
        //free(wb[i].buffer);
    }
    fclose(f2);
}

int main()
{
    FILE * f;
    SB * buffer;
    size_t bytes;
    buffer = (SB*)malloc(512);
    f = fopen("datafile-frag", "r");
    fseek(f,0,SEEK_END);
    nFileLen = ftell(f);
    fseek(f,0,SEEK_SET);
    printf("size of FS : %d\n",nFileLen / 512);
    bytes = fread(buffer, 512, 1, f);
    bytes = fread(buffer, 512, 1, f);
    printf("block size : %d\ninode_offset : %d\ndata_offset : %d\nswap_offset : %d\nfree_inode: %d\nfree_iblock : %d\n", buffer -> size, buffer -> inode_offset, buffer -> data_offset, buffer -> swap_offset, buffer -> free_inode, buffer -> free_iblock);
    printf("sizeof inode : %d\n",sizeof(IN));
    IN *inode;
    inode = (IN*)malloc(sizeof(IN));
    int i;
    for(i=0;i < 20; i++)
    {
        rb_num = 0;
        fread(inode, 100, 1, f);
        if(inode -> nlink == 0)
        {
            continue;
        }
        int sizeb = (inode -> size % 512 == 0) ? (inode -> size /512) : ((inode -> size / 512) + 1);
        printf("number :%d , nlink : %d , file size : %d\n",i,inode -> nlink, sizeb);
        int ab[512];
        int j;
        for(j=0;j<N_IBLOCKS;j++)
        {
            printf("iblock[%d] = %d\n",j,inode -> iblocks[j]);
        }
        printf("--------------\n");
        for(j=0;j<N_DBLOCKS;j++)
        {
            printf("dblock[%d] = %d\n",j,inode -> dblocks[j]);
        }
        read_and_write_file(buffer, inode , f);
    }
    printf("%d",i);
    return 0;
}
