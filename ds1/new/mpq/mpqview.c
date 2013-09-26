#ifdef WIN32
#pragma warning (push)
#pragma warning (disable  :  4057 )
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpqtypes.h"
#include "dclib.h"

#include "../structs.h"
#include "../error.h"


extern UInt32 ExtWavUnp1 (UInt32,UInt32,UInt32,UInt32); // Call for metod: 0x01
extern UInt32 ExtWavUnp2 (UInt32,UInt32,UInt32,UInt32); // Call for metod: 0x40
extern UInt32 ExtWavUnp3 (UInt32,UInt32,UInt32,UInt32); // Call for metod: 0x80

//  declaration of functions
int    test_prepare_archive (void);
DWORD  test_tell_entry      (char * filename);

void   BuildBaseMassive     (void);
int    InitializeLocals     (void);
void   FreeLocals           (void);
DWORD  Crc                  (char * string, DWORD * massive_base, DWORD massive_base_offset);
void   Decode               (DWORD * data_in, DWORD * massive_base, DWORD crc, DWORD lenght);
UInt16 read_data            (UInt8 * buffer, UInt16 size, void  * crap);
void   write_data           (UInt8 * buffer, UInt16 size, void  * crap);
int    ExtractTo            (FILE * fp_new, DWORD entry);
int    ExtractToMem         (void * mp_new, DWORD entry);

// a struct typedef
typedef struct params
{
    char * buf_in;
    char * buf_out;
} params;

GLB_MPQ_S * glb_mpq = NULL; // pointer to current mpq file structure


// ==========================================================================
// read from mod directory, NOT from a mpq
int mod_load_in_mem( char *moddir, char *filename, void **buffer, long *buf_len, int output )
{
    FILE       *in          =   NULL;
    long        size        =   0;
    char        strtmp[256];
    int         length;

    // 初始化
    *buffer     =   (void*)NULL;
    *buf_len    =   0;

    DEBUG_MESSAGE("mod_load_in_mem(\"%s\",\"%s\"):in\n", moddir, filename);
    // open file
    sprintf(strtmp, "%s/%s", moddir, filename);
    length = strlen( strtmp );
    length--;
    while( length >= 0 ){
        strtmp[length] = tolower( strtmp[length] );
        if( strtmp[length] == '\\' ){
            strtmp[length] = '/';
        }
        length--;
    }

    // strtmp == "\\Data\\Obj.txt"
    in = fopen(strtmp, "rb");
    if (in == NULL){
        return -1; // not read
    }

    // get size, for malloc()
    fseek(in, 0, SEEK_END);
    size = ftell(in) + 1;
    fseek(in, 0, SEEK_SET);
    *buffer = (void *) malloc(size);
    if (*buffer == NULL) {
        fclose(in);
        FATAL_EXIT("not enough mem (%li bytes) for %s (from Mod Directory)", size, filename);
    }
    size--;
    *buf_len = size;

    // copy the file in mem
    fread(* buffer, size, 1, in);
    * ( ((char *) (* buffer)) + size) = 0;

    //end
    if( output ){
        DEBUG_MESSAGE("ok (%li bytes)\n", size);
    }
    fclose(in);
    DEBUG_MESSAGE("mod_load_in_mem(\"%s\",\"%s\"):out\n", moddir, filename);
    return 0; // not equal to -1 means it's ok
}


// ==========================================================================
void mpq_batch_open(char * mpqname)
{

    // in case
    if(glb_mpq == NULL){
        FATAL_EXIT("pointer glb_mpq == NULL\n");
    }
    if(glb_mpq->is_open != FALSE){
        FATAL_EXIT("glb_mpq->is_open != FALSE\n");
    }
    if(mpqname == NULL){
        FATAL_EXIT("mpqname == NULL\n");
    }

    // ok, let's go
    glb_mpq->avail_metods[0] = 0x08;
    glb_mpq->avail_metods[1] = 0x01;
    glb_mpq->avail_metods[2] = 0x40;
    glb_mpq->avail_metods[3] = 0x80;

    // open mpq
    strcpy(glb_mpq->file_name, mpqname);
    glb_mpq->fpMpq = fopen(glb_mpq->file_name, "rb");
    if( ! glb_mpq->fpMpq){
        FATAL_EXIT("could not open %s\n", glb_mpq->file_name);
    }

    // prepare mpq to be read
    if (test_prepare_archive()){
        exit(DS1ERR_INITMPQ);
    }

    // flag this mpq to have some memory to free at exit
    glb_mpq->is_open = TRUE;
}


// ==========================================================================
int mpq_batch_load_in_mem(char * filename, void ** buffer, long * buf_len, int output)
{
    int  num_entry, size_unpack;
    void * mp_new;
    char strtmp[256];

    // in case
    if (glb_mpq == NULL) {
        FATAL_EXIT("mpq_batch_load_in_mem_mpq() : pointer glb_mpq == NULL");
    }
    if (glb_mpq->is_open == FALSE) {
        FATAL_EXIT("mpq_batch_load_in_mem_mpq() : glb_mpq->is_open == FALSE");
    }
    if (filename == NULL) {
        FATAL_EXIT("mpq_batch_load_in_mem_mpq() : filename == NULL");
    }
    if (strlen(filename) == 0) {
        FATAL_EXIT("mpq_batch_load_in_mem_mpq() : strlen(filename) == 0");
    }

    // let's go
    num_entry = test_tell_entry(filename);
    if (num_entry != -1) {
        //hash表指示出的是该文件在block表中的位置?
        //table每个项的第三个是size
        size_unpack = * (glb_mpq->block_table + (num_entry * 4) + 2);
        if(num_entry < 0 || num_entry > (int) glb_mpq->count_files - 1) {
            //参数检查而已
            FATAL_EXIT("Error! Invalid entry number %i (Valid entry numbers are 1-%i)\n", num_entry + 1, glb_mpq->count_files);
        }else{
            // load the file into memory
            mp_new = (void *) malloc(size_unpack);
            if (mp_new == NULL) {
                FATAL_EXIT("mpq_batch_load_in_mem() : not enough memory for %i bytes\n", size_unpack);
            }

            //查表, 然后直接在这里解压缩
            //解压缩到mp_new中, 并通过buff返回
            //size_unpack是未压缩时的数据长度
            ExtractToMem(mp_new, num_entry);
            * buffer = mp_new;
            * buf_len = size_unpack;
            if (output)
                printf("ok (%i bytes)", size_unpack);
        }
    }
    return num_entry;
}


// ==========================================================================
void mpq_batch_close()
{
    fclose(glb_mpq->fpMpq);
    FreeLocals();
}


// ==========================================================================
// open and prepare the archive
int test_prepare_archive(void)
{
    DWORD   mpq_header[2] = {0x1a51504d, 0x00000020};
    DWORD   detected = 0;
    DWORD   tmp;
    char  name_htable[] = "(hash table)";
    char  name_btable[] = "(block table)";

    // search in the mpq the bytes 4D 50 51 1A 20 00 00 00
    //     = search the string : "MPQ" + 0x1A + " " + 0x00 + 0x00 + 0x00
    while(fread(&tmp,sizeof(DWORD),1,glb_mpq->fpMpq)) {
        if(mpq_header[0]==tmp) {
            fread(&tmp,sizeof(DWORD),1,glb_mpq->fpMpq);
            if(mpq_header[1]==tmp) {
                detected=1;
                break;
            }
        }
    }
    if(!detected) {
        printf("\nError: File \'%s\' is not a valid MPQ archive", glb_mpq->file_name);
        fclose(glb_mpq->fpMpq);
        return -1;
    }

    // found the start of mpq infos in the archive
    // it might be the start of archive
    glb_mpq->offset_mpq = ftell(glb_mpq->fpMpq) - 8;

    // read 4 bytes : Lenght of MPQ file data
    fread(&glb_mpq->lenght_mpq_part,sizeof(DWORD),1,glb_mpq->fpMpq);

    // SKIP 4 bytes
    fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq + 16,SEEK_SET);

    // read 4 bytes : Offset to hash_table of MPQ
    fread(&glb_mpq->offset_htbl,sizeof(DWORD),1,glb_mpq->fpMpq);

    // read 4 bytes : Offset to block_table of MPQ
    fread(&glb_mpq->offset_btbl,sizeof(DWORD),1,glb_mpq->fpMpq);

    // read 4 bytes : Lenght of hash table
    fread(&glb_mpq->lenght_htbl,sizeof(DWORD),1,glb_mpq->fpMpq);
    glb_mpq->lenght_htbl *= 4;

    // read 4 bytes : Lenght of block table
    fread(&glb_mpq->lenght_btbl,sizeof(DWORD),1,glb_mpq->fpMpq);

    // Number of files in MPQ (calculated from size of block_table)
    glb_mpq->count_files = glb_mpq->lenght_btbl;
    glb_mpq->lenght_btbl *= 4; // in fact, length of block table is in # of file

    // prepare Crc
    // 就是一个加密解密表而已...可以手工计算一下
    //我已经算过了,在这个程序里面
    BuildBaseMassive();
    // 这个函数中就是分配一些内存而已~
    // 没有做其他的事情
    if(InitializeLocals()) {
        fclose(glb_mpq->fpMpq);
        return -2;
    }

    // read hash table
    fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+glb_mpq->offset_htbl,SEEK_SET);
    fread(glb_mpq->hash_table,sizeof(DWORD),glb_mpq->lenght_htbl,glb_mpq->fpMpq);

    // read block table
    fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+glb_mpq->offset_btbl,SEEK_SET);
    fread(glb_mpq->block_table,sizeof(DWORD),glb_mpq->lenght_btbl,glb_mpq->fpMpq);



    //char  name_htable[] = "(hash table)";
    //char  name_btable[] = "(block table)";
    //  直接对这两个字符串进行计算???????????????????????????
    // calculate crc of "(hash table)", then decode its datas
    // calculate crc of "(block table)", then decode its datas
    // hash表的内容也被加密了, 需要使用解密算法(等长加密和等长解密)
    tmp=Crc(name_htable,glb_mpq->massive_base,0x300);
    Decode(glb_mpq->hash_table,glb_mpq->massive_base,tmp,glb_mpq->lenght_htbl);
    tmp=Crc(name_btable,glb_mpq->massive_base,0x300);
    Decode(glb_mpq->block_table,glb_mpq->massive_base,tmp,glb_mpq->lenght_btbl);

    // end
    return 0;
}

// ==========================================================================
// search which entry in the archive is the file
//输入就一个字符串
//查hash table, entry记录在table中
DWORD test_tell_entry(char * filename)
{
    DWORD scrc1, scrc2, scrc3, pointer_ht;

    scrc1 = Crc(filename, glb_mpq->massive_base, 0);
    scrc2 = Crc(filename, glb_mpq->massive_base, 0x100);
    scrc3 = Crc(filename, glb_mpq->massive_base, 0x200);
    //运算结果在这里
    //如果冲突了就往后面移动一个位置
    pointer_ht = (scrc1 & (glb_mpq->lenght_htbl/4-1) ) * 4;
    //DWORD  lenght_htbl;        // Lenght of hash table
    for(; pointer_ht<glb_mpq->lenght_htbl; pointer_ht += 4)
    {
        //测试,后移动
        if( (* (glb_mpq->hash_table+pointer_ht)   == scrc2) && (* (glb_mpq->hash_table+pointer_ht+1) == scrc3)){
            //* (glb_mpq->hash_table + pointer_ht + 2)是什么东西?
            //这里+0和+1是效验码,+3是entry,2是啥?
            //一直找不到2是什么, 貌似也没有用到
            return * (glb_mpq->hash_table + pointer_ht + 3);
        }
    }
    return (DWORD) -1;
}


/******************************************************************************
 *
 *   FUNCTION:   InitializeLocals() - Allocation of memory for hash_table,block_table,
 *                                    filename_table,identify_table and working buffers 
 *                                    to decompress files
 *                                     
 ******************************************************************************/
int InitializeLocals()
{
    //看到这里, 没有继续看
    //MPQ部分明显不是作者写的, 风格完全不一样
    //从另一个侧面也说明即使不用看MPQ的实现细节
    //也可以吧这个程序做出来
    glb_mpq->global_buffer = (UInt8 *) malloc(0x60000); // Allocation 384 KB for global_buffer
    if(!glb_mpq->global_buffer) {
        printf("\nError! Insufficient memory");
        return -1;
    }
    glb_mpq->read_buffer_start=glb_mpq->global_buffer;                  // 4 KB for read_buffer
    glb_mpq->write_buffer_start=glb_mpq->global_buffer+0x1000;          // 4 KB for write_buffer
    glb_mpq->explode_buffer=glb_mpq->global_buffer+0x2000;              // 16 KB for explode_buffer
    glb_mpq->file_header=(DWORD *)(glb_mpq->global_buffer+0x6000);      // 360 KB for file_header (max size of unpacked file can't exceed 360 MB)


    glb_mpq->hash_table=(DWORD *)malloc(glb_mpq->lenght_htbl*4);
    glb_mpq->block_table=(DWORD *)malloc(glb_mpq->lenght_btbl*4);
    glb_mpq->filename_table=(char *)calloc(glb_mpq->lenght_btbl/4,MPQTYPES_MAX_PATH);
    //glb_mpq->identify_table 是个神马东西???????????
    glb_mpq->identify_table=(char *)calloc(glb_mpq->lenght_btbl/4,sizeof (char));
    if(glb_mpq->hash_table && glb_mpq->block_table && glb_mpq->filename_table && glb_mpq->identify_table)
        return 0;
    else {
        printf("\nError! Insufficient memory");
        return -1;
    }
}

/******************************************************************************
 *
 *   FUNCTION:   FreeLocals() - free memory
 *                                     
 ******************************************************************************/
void FreeLocals()
{   
    if(glb_mpq->global_buffer)
        free(glb_mpq->global_buffer);
    if(glb_mpq->hash_table)
        free(glb_mpq->hash_table);
    if(glb_mpq->block_table)
        free(glb_mpq->block_table);
    if(glb_mpq->filename_table)
        free(glb_mpq->filename_table);
    if(glb_mpq->identify_table)
        free(glb_mpq->identify_table);
    return;
}

/******************************************************************************
 *
 *   FUNCTION:   BuildBaseMassive() - fill massive_base
 *
 ******************************************************************************/
void BuildBaseMassive()
{

    //这个东西是固定的, 完全可以自己计算出来 {{{
    //我把这个表计算了一下
    //大概是如下这般, 比较长(1280个DWORD):

    // 1439053538    46006640  1481339348   696578062  3062024201  2446296939  2002426916  1501387941  1511718578  3555315213   852348408  2978087373  1020134862  2774340542  3138059889  2474027827  2223989854  2279889477  1169711127  1583705256  
    //  461707925   625138647  2874014180  1500950171  1815182973  3453824916  1010570213   861606769   832934602  1373961292  4158348079  2327690817  2347411060   550941465  3731659421  3316417765  1146773483  2492828848   511522887  4082969008  
    // 3395079898  3073558709  4258021853   249359382  2634153646   291885966  2665583560  3207835964  1639714300  4013849577  4174697669  3348854420  1512397123   188600842  2635773347  3483941463  1584230505  3414800177  1036046683   223182462  
    // 2548268843  4277939121  4183496373   665148307  1173497641  1285056048  2087086178  1604585543  2750505477  2031843172   946550486  1852588015  3344455131  1253866951  1244827547  3471969295  1622707975  4156246927  2198557074  2499219948  
    //  539297618   966599075  3854599181  3882680229  1649388329  1576088550  2212542904  3313069051  1249555152  3126437921    61349452  3443187675   850553740  2385888563  2657288425  2365139653  2282559723   320492467   785570916  2989643713  
    // 2690928833   462605198  1928164110   430343664  2189649027  1121537674  4003190518  1433209706  3127203994  3726953700  1395826540  3344236171  1346186695   268598180  3551922827  1759939971   177285289  1499439967   184792510   352994756  
    // 2976029902  3858188667  2322634916  3089317481  2392914296  1532467568  3937485592  1260958307  3061214447   108537975   358378960  3339793212  4162440927    15408488  3607100014  1134451042  3613775579  3406548713  1832670791  2522113779  
    //  212435913  1961673166  4228211638  1118867438  3316877395  4266667243  2068000687  3272205840   741971928  1131890260   364748076  3983163917  3355398668  3098116049  1582129356  3111357713  3659629055  1940661655   982445197  2894456697  
    // 4093212118  3120462737   253058334   677426909  2203678667  2959111222  2628484971  2327384454  3240841705   537415307  1153076909  2772611376  2193413572  3430186830   253299160  3659804188  1870469792  3529663622  2295363614  2875808973  
    // 4199977285  3036591452  3192383638  3876223624  3579938210  2901723249   153625394   501148328  1493990107  4104746599  3218626309   245857484  2851623780  2199476329  2348483610  3682873180  3606115093  2809491111   173126713  2850945187  
    // 3641200175  2188642296  3568075396   158571789  3015404796  3577333629  1807522496   902185386  2471270064  3514495767   700889839  2994064812  3609617118  2673616196  3249114997  3649998746  1203592326  1892028582  3041406676   599377548  
    // 1008447078  1767206527  3640849888  1698503003  3731177979   129812140  3337516936  2148873428   740242880  3655229773  3184307333  3033833690   606950555  2643871638  3117332866  1509595561  3633867969  2660286881    29087791  3622946301  
    // 3716447930  2285711453   636563794  1251109197  2012341761  2149902055    44430653  2621371609  2940222668  3802989504  1190000364  1872483302  3424955770  1202694930  2183761399  1888263916  1996014001  3012843986  1058940633  3783334887  
    // 1458379853  3385953016  3894937102  2445990446  1556871607  4054143700  1016939331  3774908293   719318719  4177674400  1023986919  1114533808  1463588943   329794478  1080783929  1667270105  1342225050  2907326379  2936020365  1100548014  
    //  408719254  3857313101  1635139891  3547348375  2895704262  3687031676  2247102660  2420710962  4046330047  3364656865  3556497113  3821177608  3586066636  2984740953   625401295  2955981312  2868717585  1437740321  3549164903  1184660011  
    // 1523603346  2711984158  2113635204   886163978  2125519840  3592216822   964716727  3862981919  2511235911  3827831344  2961453153  2802465384  3812685349  1927069693  2394468301   414606879   401562204      809762  1805727798  1634067331  
    // 1950882907  3849258769  2725204004  1609707104   832956457  2701741040  3330337900  2364745687  3958453386  1780645167  1946571140  2381730066   733808110  3838840534   696315418  1793186419  3896906970  1821968043  3482190555  3547479703  
    // 2465623215  2894369223  2932233878  3112802255  4195796834  1836303967   542974630  1305717413   770403264  2671208556  1906649131   929806816  2147428891  3731331119  2629776334   778939191  1848911033   249643964  2200592601  3143838177  
    //  334937853  1499855748  4127925052  1030312323  1127359649  3050949349  2185599915  2669785873  2943111751  3024597306  2066118339  3280566733  2233904677  3310398811  1750112679  1709818557  3855014966  3775608745  2281793656  3464615140  
    //  618069120  1090917677  1833918352  3302782142  3678955372  1823828407  4089403791    69929177    46969566  2969923429  1201228532  4195621796  2474027822   128411447  1978132220  1151172769    62334365  4141823370  1517671865  1193830691  
    //  991572207  4005444970  2645666304     8032473   610693233    27030479  1761844173  2509244228  1696117227  1761997314  3996733874  3779198121  2038409266  1519773019  3592829717  1533146004  2306525980     6259625   574754673  4103411566  
    // 2115211069   328393604   926676906  2387683350  2841686986    74766156  4289517271   787869086  1620650666  2120814233   599815325  3638726917  1432181001   358816766  2795264540  1343231946  2571337800   947075783  3867337416  3563413462  
    // 2507033604   880670275   793450363  2481797796  2810060245  2061850370  3534041049  4018905484  3084108270  2155395725  2874845915  1965809807   109785537   126529084  2858058563   306025064   753593923  2394796601  1694650867    76035656  
    // 2355290414   313729383  3843108506   268445000  1048215949   125719335  4047555617  1827133277  2610559397   840266780  3064650670  1938254044  2191750191  1985048608  3716623064  2634613426  2098883332  1228412306  1671844517   202761766  
    // 3794759894    31998853  2996691252   549781518  3915510851   758212142  1021754408   888790420  4198882899  1232439485  2489852251   125303388  1869681856  3773310612  3828816262  1460743650  3771384457  4070821600  1782702521  3217991549  
    // 4180147538  1769242008  3198643419  2379453783    81485518   705201608  1004857582   567772636   165575694   844687923  1368533378  4002271268  1039570525   817963899  4160120938  2678890886  2084569206  2242703242  4105622048  3720562716  
    // 1134472900  1903935075  2186934951  3598892392  2469037584   952044159  1645404871  2297311592  3015864391  1630784286   355030278  1525047878   488672733  2837265850  3272971917  1789027915  1902972106  2300528982  2156293119  3176581191  
    // 2425569882   685612679  2230271496  1762281942  1786707842  2882528256  1027335626   941319436  1266452002  1040336627  2740328020  2491975260   988748786  1583179938  3543080412  1368861680   393879867  1184069058   929303504  4231210184  
    //  118037001   761517051   559149091  1210202229  1638225958  2350081336  1240931736  2227688734  2236947059   332793029  3607209510  2277000394  1948168861   707565436  3348613635  1816167895   766310299  1934598891  3425240407  4196300260  
    //  112368167  3201182330  1889817931  3375731758  2808396737   535007702  2013786306  1080061657  3843371142  3911024082  3982310313  2853484106  2209500633  3961517517  3123964627  3840897884  3154519096  3636450595  1195187716   537393319  
    // 2955477997  2263517914  2511038927  4246706185  2629032123  1374968152  2631746166  3259336286  2399961963  1323817991  4130770333  4012645825  3998659915  3177369135  4103170733  3357324692  1098030921   616493220  3740567465   573441464  
    // 2369516998   297095177  2903102131   433823697  3611455621   870055051  1382781816  2030967752  2996406784  2774800131   108450377  1804699084   820262031  2193960805  1472715812  1218191053  2680860714  1456869655  4270234796  1442599232  
    // 2143007618  2656763001   268729497  4238301572  1448640045  3112276976  2566807182   699489131  2506223701  3138169381   508480533   603273531   138785880  3472844746  2962853900    11621967   971961297  4240555908  2894631861  1686421262  
    // 2445290163  2349599856  1752148166  3782328022  3752736735  3584140519  3823847841  1583355104   542843308  4250448858   779880397  4012689498  3612900166  4095181949  2318454465  1889029996  4038472584  3256928653  3991284022  2462099379  
    //  519270888   537261999  2443473509  2966377732  3573590952  3411823571  2604387267  3479585973  3465840879     4180334  2856460847  1212478416  1303944567   141040302  1714371112  1981108955   502767994  2482957722  3427363407  2719469517  
    // 1000589616   646938231   127382689  2347498577   109654212   382235874  3259161124  4213350262  2692307714  1755299894   396834610  3524979712  2885723794  1528921880  2923566628   734026944  1220664186  1330537396   375910478   872550292  
    // 2536187221  2584163675    23484784  2109082735   987172923  1547504064  4003715802  2294422544   270808791  1782264832  3948035148  4221032598  2521610347  3716644920    86628941   185886885   410229448  4159858280  4132390001  1544877507  
    // 1151369723   967605815  2316812907  2257542759   959551373  1860139031  1230097511  2486437875  3428742287   729737001  3417514224   301910247  4241475160    54870868   926589420   492371610   105320622  2827657402  3515064894  1087459491  
    // 4113676497  2816057221   293833941  2979947699  1030202857  2955696838  2579085800  1277176698   368206291  2823564557  1358005670   867844565   964235283  1269822542  4008618524  3762848532  1436339499  2701456453  1668233156   642604603  
    // 1975615285  1967276337  3138475833  1962833242  2100984486   764143492  2213352778  4230378456  1155353215  2030398664  2194442289  2637962108  3913628666   607213204  1713320531  1968480092  3140183021  1742736790  4102645447   226224726  
    // 3261043439  2840045469  3729448815  2886665026  3555402859  2762609038  1010723398  1165968449  2768912495  1817218463  3920369895   516797626   967605818  2142438650  1866004712  1257040493  1627610739  2171635975  3955257861  2432529940  
    // 3039633791  1817043349   500469892  1693337644  2496111905  1410075096  1977585109  1094791671   915996022  4152679247  4173953539    76495289  1005908157  2360674706  2606291438  3853220300  2589941842  3180695984  4208075563  1516446292  
    //   42395160  2329135351  3825598838  1967932951  1544571147  1870491647   562541610  1467287899    49596003  1516096006  4266316999  1407295357  1509902005  3166972837  4247734935  3201751301  2506267499   564248795  2853221469  3874450776  
    //  680709965  2683137040   507014135  2615199464   724506095   435530875  2063535730   836392815   807742644  1467572396   376348250  2838973025  1899404438  2146969262  3893842726  3401317683    26286350  1771912254   917768994  3012887827  
    // 2609421306  2771013639  2888372205   994986575  3075725526  1012080418  1479500839  2848822185  1416509870   607103698  3537958727   167129693  1507100441  2910762606  3578865784  1457438621  1359472037  4055894692  2169293992  1611501808  
    // 2615768601  2438964722   727066845  2738708314  1499330497  1768300893  1357677367  4057076594   249753339  2912841911  2869768164  3475471197  1649957420  2203744324  1957821035  1236991957   782878820   311628200  3187284071  2211076503  
    // 2635970329  2226878950  2654968306   826237185   463021147  3512569778  3472428929   692397709  3686593907  2969704582  2665999389   486024439  2125082162  1774232245  1955873180  3797145555  1222262034  1169732977  3679261818  3626864094  
    //  643633314  2963729306  2115802022   922627910  3702352640    88314257  2966312068  1641136905  2376477187   114359855  3133594940  3391096563  2200023471    68265715  3901853356   564314450  2445333839  1789531386  3710035008  2295013493  
    // 2051979283  1739563121  3638135964   837881159  1878108328   799731873  1218585019  1004967051  2860115867   245835660  2050512879  1236794965  3575429425    68944269    12125441  1172994177  2411846607  3087741609  2880230121  1736170560  
    // 3970753858  1835341035   834707447  1584143037   473505039   245200881  1897478445  1810477242  1637153399  1172709704  1109609202  3273891174  2476522940  1444941212  1797739016     1729011  3435308358  1265773536  3995464375  1837551518  
    // 2859196621  1305323446  2027203091   921336537  1059706624  3739889028  1785394621   414169116  3638880060  2326202554  1703646425  2792769420  3483438114   626605172  2375426613  1603797616  1039592384  2390594298  3287176673  3589743687  
    //   45152917  1387312416  1892860191   980409708   423274136  1419179989  1107858203  2051081964  2213046336  4103827340  3124599377  1442139613   769987442  2600163099   984830982  2909077279  3346490622  2139505766  1097702612  2722665080  
    // 3809183460   189936054  3127357142  3853198439  1610779536   496902342   712796458  1802050779  2656172055  2608217580  1026110015  3487640424  1967867291  2720235591  3653588244  1809645505  1765696320  2184724455   420932191  3967536473  
    // 1927157338  3885635095  2395497012  2834354820   550044198  2079950980   681935665  1053031022  1289783643  3277042896   871302735   568713744  3367721117  3898460850  2892333727  2814350032    94771021   631573550  3855058807  1031078431  
    // 1269472408   599640196  2522069975  3135039477  3263407236  3225476910  2473633857   791765026   921621041  2888612876   331392155  3453036951   134123954  3447630797   738273053   407055864  1503423431  1978219866  3061477090  2389937686  
    // 4287656957  2943812202  4168000252  2437979811   866618822  1333338954   980759996   892992689   822341202  1332682345  1806953404  1287091541  2379344281   719209373   290594734  3108840773  2380176061   420034879  3079840317  1956398427  
    // 3048607532   939809230   229464096  2972746843   689749330  3734657975  1373567329  1364943880  3396239934   942567028  4005444977  1284443242  3866549482   350346364  1870644788  3481643331  3550390638   326007955  1902424836  4079948595  
    //  901988404  3728332577  2443736121  3289387295   703691436   320251647   317822190  1108055193  4146966748  1762369427   399592370  2408782520   561316028  2270434296  2561860669  3307181425  1422835146   119131257  4089907213  3827853169  
    // 2750067661  1248526560   298780505  2122937209  1530038124  1586287862   216922724  2442838727  2987717555   243909516  4039391801  3940615458  2203831814  2666283970  3928402390  1767053345  1730523722  3665932514  3585388083   135240187  
    //  363347375   702531360  2447478799   672546136  2211514274  1319900314  2675038744  1641399546  3819645535  3350255167  3016521002  1642844086   654292266  3984105023  2856044888  1163560840  3384836777   169602830  3264895614   353191733  
    // 2486700512  1763792144  4010479017   995008435  3635793996  3349160872  2966749832  1686246261  3261130959  2224471340  4283388981   263542284  2094484050  2472057984  2291620935  2210244765  2345419414    28847137  4253162943   642911047  
    //   77086233  3554789927  2016172047   846198131  3243752647  1331587972  1156316155  3659979224  1826214030   387576280  1901855879   741162130  1408936873  3162726726  1359778518  3767138346  1182799647   916390110  1894523697   237321557  
    // 3673921303  3856503341  2509331745  2686463894  2707541063  3939477233  3982397833  3669522052  2763484490  4201662615  1662980282  3550324985  1572126948   385737770   771716480  3044011134  3288314736  2389653180  1620563033  1364024630  
    // 2779199458   964147635  4247888074  2419288283  3895922017  2551223584  2917285036  2583178756  1105100451  2225193621   152640480   814943490   480224330  3288774359  4007436626  1632207004  2953420532  3321254821  1276147981  1929586796  
    // }}}
    DWORD   s1;
    int i,j;
    ldiv_t divres;

    divres.rem=0x100001;
    for(i=0;i<0x100;i++) {
        for(j=0;j<5;j++) {
            divres=ldiv(divres.rem*125+3,0x002AAAAB);
            s1=(divres.rem&0xFFFFL)<<0x10;
            divres=ldiv(divres.rem*125+3,0x002AAAAB);
            s1=s1|(divres.rem&0xFFFFL);
            glb_mpq->massive_base[i+0x100*j]=s1;
        }
    }
    return;
}

/******************************************************************************
 *
 *   FUNCTION:   Crc(char *,DWORD *,DWORD) - calculate crc
 *   说白了就是那个算hash值
 *
 ******************************************************************************/
DWORD Crc(char *string,DWORD *massive_base,DWORD massive_base_offset)
{
    char   byte;
    DWORD  crc=0x7fed7fed;
    DWORD  s1=0xEEEEEEEE;

    byte=*string;
    while(byte) {
        if(byte>0x60 && byte<0x7B)
            byte-=0x20;
        //massive_base其实就是个加密用的查找表
        crc=*(massive_base+massive_base_offset+byte)^(crc+s1);
        s1+=crc+(s1<<5)+byte+3;
        string++;
        byte=*string;
    }
    return crc;
}

/******************************************************************************
 *
 *   FUNCTION:   read_data(UInt8 *,UInt16,void *) (called by explode)
 *
 ******************************************************************************/
UInt16 read_data(UInt8 *buffer,UInt16 size,void *crap)
{
    params *param=(params *)crap;
    memcpy(buffer,param->buf_in,size);
    param->buf_in+=size;
    return size;
}
/******************************************************************************
 *
 *   FUNCTION:   write_data(UInt8 *,UInt16,void *) (called by explode)
 *
 ******************************************************************************/
void write_data(UInt8 *buffer,UInt16 size,void *crap)
{
    params *param=(params *)crap;
    memcpy(param->buf_out,buffer,size);
    param->buf_out+=size;
    glb_mpq->lenght_write+=size;
}


/******************************************************************************
 *
 *   FUNCTION:   Decode(DWORD *,DWORD *,DWORD,DWORD) - decode data
 *
 ******************************************************************************/
void Decode(DWORD *data_in, DWORD *massive_base, DWORD crc, DWORD lenght)
{
    //传入: 
    //      data_in      = hash表首地址
    //      massive_base = 加密表
    //      crc          = "(hash table)"这个字符串的hash值
    //      length       = hash表的长度
    DWORD  i,dec;
    DWORD  s1=0xEEEEEEEE;
    //长度不变的decode
    for(i=0;i<lenght;i++) {
        s1+=*(massive_base+0x400+(crc&0xFFL));
        dec=*(data_in+i)^(s1+crc);
        s1+=dec+(s1<<5)+3;
        *(data_in+i)=dec;
        crc=(crc>>0x0b)|(0x11111111+((crc^0x7FFL)<<0x15));
    }
    return;
}

/******************************************************************************
 *
 *   FUNCTION:   GetUnknowCrc(DWORD) - calculate crc for file without name
 *
 ******************************************************************************/
DWORD GetUnknowCrc(DWORD entry)
{   
    DWORD   tmp,i,j,coded_dword,crc_file=0;
    DWORD   flag,size_pack,size_unpack,num_block,offset_body;
    DWORD   sign_riff1=0x46464952; // 'RIFF'
    DWORD   sign_riff3=0x45564157; // 'WAVE'
    DWORD   sign_mpq1=0x1a51504d; // 'MPQ'
    DWORD   sign_mpq2=0x00000020;
    ldiv_t  divres;

    offset_body=*(glb_mpq->block_table+entry*4);                                // get offset of analized file
    flag=*(glb_mpq->block_table+entry*4+3);                                 // get flag of analized file
    fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
    fread(&coded_dword,sizeof(DWORD),1,glb_mpq->fpMpq);                     // read first coded dword from file

    if(flag&0x200 || flag&0x100) {                              // IF FILE PACKED:
        size_unpack=*(glb_mpq->block_table+entry*4+2);                      // . get size of unpacked file
        size_pack=*(glb_mpq->block_table+entry*4+1);                            // . get size of packed file
        divres=ldiv(size_unpack-1,0x1000);
        num_block=divres.quot+2;                                    // . calculate lenght of file header
        for(j=0;j<=0xff;j++) {                                      // . now we're gonna find crc_file of 0x100 possible variants
            crc_file=((num_block*4)^coded_dword)-0xeeeeeeee - *(glb_mpq->massive_base+0x400+j);// . calculate possible crc
            if((crc_file&0xffL) == j) {                             // . IF FIRST CHECK is succesfull - do second one
                fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
                fread(glb_mpq->file_header,sizeof(DWORD),num_block,glb_mpq->fpMpq); // . read file header 
                Decode(glb_mpq->file_header,glb_mpq->massive_base,crc_file,num_block);// . decode file header with possible crc
                tmp=num_block*4;                                    // . tmp = size header (bytes)
                if(tmp == *glb_mpq->file_header) {                          // . IF SECOND CHECK is succesfull - do third one
                    for(i=0;i<num_block-1;i++) {
                        tmp+=*(glb_mpq->file_header+i+1)-*(glb_mpq->file_header+i);
                        if(*(glb_mpq->file_header+i+1)-*(glb_mpq->file_header+i)>0x1000) {
                            tmp=0xffffffff;
                            break;
                        }
                    }
                    if(tmp!=0xffffffff) {                           // . IF THIRD CHECK is succesfull
                        crc_file++;                                 // . great! we got right crc_file
                        break;
                    }
                }
            }
            crc_file=0;                                             // . if its impossible to get right crc return 0
        }

    } else {                                                    // IF FILE IS NOT PACKED:
        for(j=0;j<=0xff;j++) {                                      // Calculate crc as if it was WAV FILE
            crc_file=(sign_riff1^coded_dword)-0xeeeeeeee - *(glb_mpq->massive_base+0x400+j);// . calculate possible crc
            if((crc_file&0xff)==j) {                                // . IF FIRST CHECK is succesfull - do second one
                fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
                fread(glb_mpq->file_header,sizeof(DWORD),3,glb_mpq->fpMpq);         // . read file file_header 
                Decode(glb_mpq->file_header,glb_mpq->massive_base,crc_file,3);      // . decode file file_header with possible crc
                if(sign_riff1==*glb_mpq->file_header) {
                    if(sign_riff3==*(glb_mpq->file_header+2))               // . IF SECOND CHECK is succesfull - we got right crc
                        break;
                }
            }
            crc_file=0;                                             // . if its impossible to get right crc return 0
        }
        if(!crc_file) {                                             // Calculate crc as if it was MPQ FILE
            for(j=0;j<=0xff;j++) {
                crc_file=(sign_mpq1^coded_dword)-0xeeeeeeee - *(glb_mpq->massive_base+0x400+j);
                if((crc_file&0xffL) == j) {
                    fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
                    fread(glb_mpq->file_header,sizeof(DWORD),2,glb_mpq->fpMpq);
                    Decode(glb_mpq->file_header,glb_mpq->massive_base,crc_file,2);
                    if(sign_mpq1 == *glb_mpq->file_header) {
                        if(sign_mpq2 == *(glb_mpq->file_header+1))
                            break;
                    }
                }
                crc_file=0;
            }
        }
    }
    return crc_file;
}

/******************************************************************************
 *
 *   FUNCTION:   ExtractTo(FILE,DWORD) - extract file from archive
 *
 ******************************************************************************/
int ExtractTo(FILE *fp_new,DWORD entry)
{
    DWORD   size_pack,size_unpack;
    UInt8   *read_buffer,*write_buffer;
    UInt32  i,j,offset_body,flag,crc_file=0;
    UInt32  num_block,lenght_read,iteration;
    UInt8   *szNameFile;
    UInt8   metod;
    ldiv_t  divres;
    params  param;

    offset_body=*(glb_mpq->block_table+entry*4);                            // get offset of file in mpq
    size_unpack=*(glb_mpq->block_table+entry*4+2);                      // get unpacked size of file
    flag=*(glb_mpq->block_table+entry*4+3);                             // get flags for file

    //
    //
    //
    if(flag&0x30000) {                                      // If file is coded, calculate its crc
        if(*(glb_mpq->identify_table+entry)&0x1) {                      // . Calculate crc_file for identified file:
            szNameFile=glb_mpq->filename_table+MPQTYPES_MAX_PATH*entry;         // . . get name of file
            if(strrchr(szNameFile,'\\'))
                szNameFile=strrchr(szNameFile,'\\')+1;
            crc_file=Crc(szNameFile,glb_mpq->massive_base,0x300);       // . . calculate crc_file (for Diablo I MPQs)
            if((flag&0x20200) == 0x20200)                                   // . . if flag indicates Starcraft MPQ
                crc_file=(crc_file+offset_body)^size_unpack;    // . . calculate crc_file (for Starcraft MPQs)
        }
        else
            crc_file=GetUnknowCrc(entry);                       // . calculate crc_file for not identified file:
    }

    if(flag&0x200 || flag&0x100) {                          // IF FILE IS PACKED:
        divres=ldiv(size_unpack-1,0x1000);
        num_block=divres.quot+2;                                // . calculate lenght of file header
        fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
        fread(glb_mpq->file_header,sizeof(DWORD),num_block,glb_mpq->fpMpq);     // . read file header 
        if(flag&0x30000)
            Decode(glb_mpq->file_header,glb_mpq->massive_base,(crc_file-1),num_block);// . decode file header (if file is coded)
        read_buffer=glb_mpq->read_buffer_start;
        for(j=0;j<(num_block-1);j++) {
            lenght_read=*(glb_mpq->file_header+j+1)-*(glb_mpq->file_header+j);  // . get lenght of block to read
            fread(read_buffer,sizeof(char),lenght_read,glb_mpq->fpMpq); // . read block
            if(flag&0x30000)
                Decode((DWORD *)read_buffer,glb_mpq->massive_base,crc_file,lenght_read/4);          // . decode block (if file is coded)
            if(lenght_read==0x1000 || (j==num_block-2 && lenght_read==(size_unpack&0xFFF))) // . if block is unpacked (its lenght=0x1000 or its last block and lenght=remainder)
                //
                fwrite(read_buffer,sizeof(char),lenght_read,fp_new);                    // . write block "as is"
            else {                                              // . block is packed
                if(flag&0x200) {                                // . If this file is from Starcraft MPQ (or Diablo 2), then
                    metod=*read_buffer;                         // . . first byte determinates metod of packing
                    iteration=0;
                    for(i=0;i<4;i++) {                          // . . calculate number of iterations
                        if(metod&glb_mpq->avail_metods[i])
                            iteration++;
                    }
                    read_buffer+=1;
                    lenght_read-=1;
                } else {                                        // . Else: file is from Diablo I MPQ, then
                    iteration=1;
                    metod=8;                                    // . .file is compressed with DCL
                }
                write_buffer=glb_mpq->write_buffer_start;
                if(metod&0x08) {
                    param.buf_in =read_buffer;
                    param.buf_out=write_buffer;
                    glb_mpq->lenght_write=0;
                    explode(&read_data,&write_data,&param);
                    lenght_read=glb_mpq->lenght_write;
                    iteration--;
                    if(iteration) {
                        read_buffer=write_buffer;
                        write_buffer=glb_mpq->read_buffer_start;
                    }
                }
                if(metod&0x01) {
                    lenght_read=ExtWavUnp1((UInt32)read_buffer,(UInt32)lenght_read,(UInt32)write_buffer,0x1000);
                    iteration--;
                    if(iteration) {
                        read_buffer=write_buffer;
                        write_buffer=glb_mpq->read_buffer_start;
                    }
                }
                if(metod&0x40)
                    lenght_read=ExtWavUnp2((UInt32)read_buffer,(UInt32)lenght_read,(UInt32)write_buffer,0x1000);
                if(metod&0x80)
                    lenght_read=ExtWavUnp3((UInt32)read_buffer,(UInt32)lenght_read,(UInt32)write_buffer,0x1000);
                //
                fwrite(write_buffer,1,lenght_read,fp_new);
                read_buffer=glb_mpq->read_buffer_start;
            }
            crc_file++;                                         // . calculate crc_file for next block
        }
    }

    else {                                                  // IF FILE IS NOT PACKED
        size_pack=*(glb_mpq->block_table+entry*4+1);                    // get size  of file
        if(flag&0x30000)
            lenght_read=0x1000;                             // if file is coded, lenght_read=0x1000 (4 KB)
        else
            lenght_read=0x60000;                            // if file is not coded, lenght_read=0x60000 (384KB)
        if(size_pack<lenght_read)
            lenght_read=size_pack;                          // if size of file < lenght_read, lenght read = size of file
        read_buffer=glb_mpq->read_buffer_start;
        if(lenght_read) {
            divres=ldiv(size_pack,lenght_read);                 // .
            num_block=divres.quot;                              // .
        } else {                                                // .
            num_block=0;                                        // .
            divres.rem=0;                                       // .
        }
        fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
        for (j=0;j<num_block;j++) {
            fread(read_buffer,1,lenght_read,glb_mpq->fpMpq);
            if(flag&0x30000) {
                Decode((DWORD *)read_buffer,glb_mpq->massive_base,crc_file,lenght_read/4);  // if file is coded, decode block
                crc_file++;                                     // and calculate crc_file for next block
            }
            //
            fwrite(read_buffer,1,lenght_read,fp_new);
        }
        if(divres.rem) {
            fread(read_buffer,1,divres.rem,glb_mpq->fpMpq);
            if(flag&0x30000)
                Decode((DWORD *)read_buffer,glb_mpq->massive_base,crc_file,divres.rem/4);
            //
            fwrite(read_buffer,1,divres.rem,fp_new);
        }
    }
    return 0;
}

/******************************************************************************
 *
 *   FUNCTION:   ExtractToMem(FILE,DWORD) - extract file from archive
 *
 *   把entry开始处的数据解压缩到mp_new中, mp_new的长度是该段数据尚未被压缩之前的长度
 *   entry是压缩后数据在mpq文件中的偏移
 *
 *
 ******************************************************************************/
int ExtractToMem(void * mp_new, DWORD entry)
{
    //这个函数真恐怖...没看
    DWORD  size_pack,size_unpack;
    UInt8  * read_buffer,*write_buffer;
    UInt32 i,j,offset_body,flag,crc_file=0;
    UInt32 num_block,lenght_read,iteration;
    UInt8    * szNameFile;
    UInt8  metod;
    ldiv_t divres;
    params param;
    UBYTE  * buff_ptr = mp_new;

    offset_body=*(glb_mpq->block_table+entry*4);                        // get offset of file in mpq
    size_unpack=*(glb_mpq->block_table+entry*4+2);                      // get unpacked size of file
    flag=*(glb_mpq->block_table+entry*4+3);                             // get flags for file

    //size_pack=*(glb_mpq->block_table+entry*4+1);                  // get size  of file
    //这句话是后面的, 说明了[0:offset][1:unpackedsize][2:packedsize][3:flag] 这4个数据的排列和内容

    if(flag&0x30000) {                                                  // If file is coded, calculate its crc
        if(*(glb_mpq->identify_table+entry)&0x1) {                      // . Calculate crc_file for identified file:
            szNameFile=glb_mpq->filename_table+MPQTYPES_MAX_PATH*entry;         // . . get name of file
            if(strrchr(szNameFile,'\\'))
                szNameFile=strrchr(szNameFile,'\\')+1;
            crc_file=Crc(szNameFile,glb_mpq->massive_base,0x300);       // . . calculate crc_file (for Diablo I MPQs)

            /* edit by Sloan Roy 17 Nov 2002 : must be
               if ((flag & 0x20200) == 0x20200)
               instead of
               if (flag & 0x20200) */

            if ((flag & 0x20200) == 0x20200)                                    // . . if flag indicates Starcraft MPQ

                crc_file=(crc_file+offset_body)^size_unpack;    // . . calculate crc_file (for Starcraft MPQs)
        }
        else
            crc_file=GetUnknowCrc(entry);                       // . calculate crc_file for not identified file:
    }

    if(flag&0x200 || flag&0x100) {                          // IF FILE IS PACKED:
        divres=ldiv(size_unpack-1,0x1000);
        num_block=divres.quot+2;                                // . calculate lenght of file header
        fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
        fread(glb_mpq->file_header,sizeof(DWORD),num_block,glb_mpq->fpMpq);     // . read file header 
        if(flag&0x30000){
            //Decode是定长加密解密, 所以可以直接传入结构头指针
            Decode(glb_mpq->file_header,glb_mpq->massive_base,(crc_file-1),num_block);// . decode file header (if file is coded)
        }
        read_buffer=glb_mpq->read_buffer_start;
        for(j=0;j<(num_block-1);j++) {
            lenght_read=*(glb_mpq->file_header+j+1)-*(glb_mpq->file_header+j);  // . get lenght of block to read
            fread(read_buffer,sizeof(char),lenght_read,glb_mpq->fpMpq); // . read block
            if(flag&0x30000)
                Decode((DWORD *)read_buffer,glb_mpq->massive_base,crc_file,lenght_read/4);          // . decode block (if file is coded)
            if(lenght_read==0x1000 || (j==num_block-2 && lenght_read==(size_unpack&0xFFF))) // . if block is unpacked (its lenght=0x1000 or its last block and lenght=remainder)
            {
                //
                //                  fwrite(read_buffer,sizeof(char),lenght_read,fp_new);                    // . write block "as is"
                memcpy(buff_ptr, read_buffer, lenght_read);
                buff_ptr += lenght_read;
            }
            else {                                              // . block is packed
                if(flag&0x200) {                                // . If this file is from Starcraft MPQ (or Diablo 2), then
                    metod=*read_buffer;                         // . . first byte determinates metod of packing
                    iteration=0;
                    for(i=0;i<4;i++) {                          // . . calculate number of iterations
                        if(metod&glb_mpq->avail_metods[i])
                            iteration++;
                    }
                    read_buffer+=1;
                    lenght_read-=1;
                } else {                                        // . Else: file is from Diablo I MPQ, then
                    iteration=1;
                    metod=8;                                    // . .file is compressed with DCL
                }
                write_buffer=glb_mpq->write_buffer_start;
                if(metod&0x08) {
                    param.buf_in =read_buffer;
                    param.buf_out=write_buffer;
                    glb_mpq->lenght_write=0;
                    explode(&read_data,&write_data,&param);
                    lenght_read=glb_mpq->lenght_write;
                    iteration--;
                    if(iteration) {
                        read_buffer=write_buffer;
                        write_buffer=glb_mpq->read_buffer_start;
                    }
                }
                if(metod&0x01) {
                    lenght_read=ExtWavUnp1((UInt32)read_buffer,(UInt32)lenght_read,(UInt32)write_buffer,0x1000);
                    iteration--;
                    if(iteration) {
                        read_buffer=write_buffer;
                        write_buffer=glb_mpq->read_buffer_start;
                    }
                }
                if(metod&0x40)
                    lenght_read=ExtWavUnp2((UInt32)read_buffer,(UInt32)lenght_read,(UInt32)write_buffer,0x1000);
                if(metod&0x80)
                    lenght_read=ExtWavUnp3((UInt32)read_buffer,(UInt32)lenght_read,(UInt32)write_buffer,0x1000);
                //
                //              fwrite(write_buffer,1,lenght_read,fp_new);
                memcpy(buff_ptr, write_buffer, lenght_read);
                buff_ptr += lenght_read;

                read_buffer=glb_mpq->read_buffer_start;
            }
            crc_file++;                                         // . calculate crc_file for next block
        }
    }

    else {                                                  // IF FILE IS NOT PACKED
        size_pack=*(glb_mpq->block_table+entry*4+1);        // get size  of file
        if(flag&0x30000)
            lenght_read=0x1000;                             // if file is coded, lenght_read=0x1000 (4 KB)
        else
            lenght_read=0x60000;                            // if file is not coded, lenght_read=0x60000 (384KB)
        if(size_pack<lenght_read)
            lenght_read=size_pack;                          // if size of file < lenght_read, lenght read = size of file
        read_buffer=glb_mpq->read_buffer_start;
        if(lenght_read) {
            divres=ldiv(size_pack,lenght_read);                 // .
            num_block=divres.quot;                              // .
        } else {                                                // .
            num_block=0;                                        // .
            divres.rem=0;                                       // .
        }
        fseek(glb_mpq->fpMpq,glb_mpq->offset_mpq+offset_body,SEEK_SET);
        for (j=0;j<num_block;j++) {
            fread(read_buffer,1,lenght_read,glb_mpq->fpMpq);
            if(flag&0x30000) {
                Decode((DWORD *)read_buffer,glb_mpq->massive_base,crc_file,lenght_read/4);  // if file is coded, decode block
                crc_file++;                                     // and calculate crc_file for next block
            }
            //
            //          fwrite(read_buffer,1,lenght_read,fp_new);
            memcpy(buff_ptr, read_buffer, lenght_read);
            buff_ptr += lenght_read;
        }
        if(divres.rem) {
            fread(read_buffer,1,divres.rem,glb_mpq->fpMpq);
            if(flag&0x30000)
                Decode((DWORD *)read_buffer,glb_mpq->massive_base,crc_file,divres.rem/4);
            //
            //          fwrite(read_buffer,1,divres.rem,fp_new);
            memcpy(buff_ptr, read_buffer, divres.rem);
            buff_ptr += divres.rem;
        }
    }
    return 0;
}

#ifdef WIN32
#pragma warning (pop)
#endif
