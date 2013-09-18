#include "structs.h"
#include "misc.h"
#include "dc6info.h"


// ==========================================================================
void dc6_decomp_norm(void * src, BITMAP * dst, long size, int x0, int y0)
{
    UBYTE * ptr =(UBYTE *)src;
    long  i;
    int   i2, x=x0, y=y0, c, c2;


    for(i=0; i<size; i++){
        //c为控制字符:
        //	c==0x80( 0b1000,0000 ):换行(也就是x移动到行首,y移动到下一行)
        //   c最高位为1,但不是0x80( 0b1XXXXXX ):当前行跳动0b0XXXXXXX(c的低7位,最大128,够用了~)
        //  128大概3厘米吧
        //   若最高位不是1,则接下来为c个像素数据
        //
        //
        //   我在想, 干嘛不用c=0x00(0000,0000)描述换行
        //   而c=(0XXX,XXXX)描述行内跳转, c=(1XXX,XXXX)描述连续数据?
        //   这会导致一个问题:数据至少都为c=128(1000,0000)长,而实际不一定满足
        c = *(ptr ++);

        if(c == 0x80){
            //jmp code
            // 0x80: 0b1000,0000
            x = x0;
            y--;
        }else if(c & 0x80){
            x += c & 0x7F;
        }else{
            for(i2=0; i2<c; i2++){
                c2 = *(ptr ++);
                i++;
                putpixel(dst, x, y, c2);
                x++;
            }
        }
    }
}


// ==========================================================================
// cmap must point to a table of 256 bytes
void dc6_decomp_cmap(void * src, BITMAP * dst, long size, int x0, int y0, UBYTE * cmap)
{
    UBYTE * ptr =(UBYTE *)src;
    long  i;
    int   i2, x=x0, y=y0, c, c2;


    for(i=0; i<size; i++){
        c = *(ptr ++);

        if(c == 0x80){
            x = x0;
            y--;
        } else if(c & 0x80){
            x += c & 0x7F;
        }else {
            for(i2=0; i2<c; i2++){
                c2 = *(ptr ++);
                i++;
                putpixel(dst, x, y, cmap[c2]);
                x++;
            }
        }
    }
}


// ==========================================================================
//加载.dc6
int anim_load_dc6(char * name, COF_S * cof, int lay_idx, long user_dir, UBYTE * palshift)
{
    LAY_INF_S  * lay;
    int        entry, i, size, w, h, x1, y1, x2, y2;
    char       * buff;
    long       dir = 0, * lptr, offset, len;
    long       dc6_ver, dc6_unk1, dc6_unk2, dc6_dir, dc6_fpd, * dc6_fptr,
               f_w, f_h, f_offx, f_offy, f_x1, f_x2, f_y1, f_y2,
               f_len;
    UBYTE      * f_data;


    if((cof == NULL)||(lay_idx >= COMPOSIT_NB)){
        return 1;
    }
    lay = & cof->lay_inf[lay_idx];

    // load dc6 file
    //就是直接拷贝到buff吧..
    entry = misc_load_mpq_file(name, & buff, & len, FALSE);
    if(entry == -1){
        return 1;
    }

    // decode dc6 header datas
    lptr     =(long *)buff;
    // .dc6 version, must be 6
    dc6_ver  = lptr[0];
    //unknown1,2
    dc6_unk1 = lptr[1];
    dc6_unk2 = lptr[2];
    //
    dc6_dir  = lptr[4];
    dc6_fpd  = lptr[5];

    //神马? 应该是从lprt6开始了dcc的文件内容..
    //&lptr[6] := lprt+6

    //file pointer
    dc6_fptr = & lptr[6];

    if((dc6_ver != 6)||(dc6_unk1 != 1)||(dc6_unk2 != 0)){
        free(buff);
        return 1;
    }

    // valid direction ?
    if(dc6_dir != cof->dir){
        free(buff);
        return 1;
    }

    // choose direction
    switch(cof->dir){
        case  1 : dir = glb_ds1edit.new_dir1[user_dir];  break;
        case  4 : dir = glb_ds1edit.new_dir4[user_dir];  break;
        case  8 : dir = glb_ds1edit.new_dir8[user_dir];  break;
        case 16 : dir = glb_ds1edit.new_dir16[user_dir]; break;
        case 32 : dir = glb_ds1edit.new_dir32[user_dir]; break;
    }

    // decode dc6 direction

    // search the direction "box"
    //这个box是不是和dt1 tool里面那个box有点联系?
    //表示整个图有多大矩形?
    x1 = y1 = 0x7FFFFFFFL;
    x2 = y2 = 0x80000000L;
    for(i=0; i < dc6_fpd; i++){
        //这要一frame一frame的读取了...
        // get pointer to frame header
        offset = dc6_fptr[dir * dc6_fpd + i];
        if(offset >= len){
            free(buff);
            return 1;
        }
        lptr =(long *)(buff + offset);

        // update the box limits
        f_w    = lptr[1];
        f_h    = lptr[2];
        f_offx = lptr[3];
        f_offy = lptr[4];

        f_x1 = f_offx;
        f_x2 = f_x1 + f_w - 1;
        f_y2 = f_offy;
        f_y1 = f_y2 - f_h + 1;

        //这是在统计什么东东么...
        if(f_x1 < x1){
            x1 = f_x1;
        }
        if(f_x2 > x2){
            x2 = f_x2;
        }
        if(f_y1 < y1){
            y1 = f_y1;
        }
        if(f_y2 > y2){
            y2 = f_y2;
        }
    }
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;

    if((w <= 0)||(h <= 0)){
        free(buff);
        return 1;
    }

    lay->off_x = x1;
    lay->off_y = y1;

    // allocate the bitmaps
    //我勒个擦擦..分配了一大块
    size = dc6_fpd * sizeof(BITMAP *);
    lay->bmp_num = dc6_fpd;
    lay->bmp =(BITMAP **)malloc(size);
    if(lay->bmp == NULL){
        free(buff);
        return 1;
    }
    memset(lay->bmp, 0, size);

    // make the bitmaps
    for(i=0; i < dc6_fpd; i++){
        // get pointer to frame header
        offset = dc6_fptr[dir * dc6_fpd + i];
        if(offset >= len) {
            while(i) {
                i--;
                destroy_bitmap(lay->bmp[i]);
            }
            free(buff);
            return 1;
        }
        lptr =(long *)(buff + offset);

        // get frame datas
        f_w    = lptr[1];
        f_h    = lptr[2];
        f_offx = lptr[3];
        f_offy = lptr[4];


        //图像数据..长度,然后是数据
        f_len  = lptr[7];
        f_data =(UBYTE *)(& lptr[8]);

        // make a BITMAP
        lay->bmp[i] = create_bitmap(w, h);
        if(lay->bmp[i] == NULL){
            while(i){
                i--;
                destroy_bitmap(lay->bmp[i]);
            }
            free(buff);
            return 1;
        }
        clear(lay->bmp[i]);

        // decode dc6 datas
        if(palshift == NULL){
            dc6_decomp_norm(
                    f_data,              // src
                    lay->bmp[i],         // dst
                    f_len,               // length
                    f_offx - x1,         // x0
                    h - 1 + f_offy - y2  // y0
                    );
        } else {
            dc6_decomp_cmap(
                    f_data,              // src
                    lay->bmp[i],         // dst
                    f_len,               // length
                    f_offx - x1,         // x0
                    h - 1 + f_offy - y2, // y0
                    palshift             // cmap
                    );
        }
    }
    free(buff);
    return 0;
}
