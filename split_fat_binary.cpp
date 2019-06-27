/*
*function:extract armv8_64 arch Mach-O file from fat Mach-O binary
*author : xiesikefu
*date : 2019.6.27
*/
#include<stdio.h>
#include<sys/mman.h>
#include <fcntl.h>
#include<unistd.h>
#include "fat.h"
#include "mach-O-header.h"


int need_convert;

/*
    param:1.target uint32_t num
    function: convert Big Endian or Little Endian of a uint32_t num
    return: the converted num or the original num
 */
uint32_t convert_uint32_t(uint32_t target_num){
    if(need_convert==0){
        
        return target_num;
    }
    uint32_t ori_target_num = target_num;
    uint32_t result_num; 
    result_num = 0;
    for(int i=0;i<4;i++){
        result_num = result_num << 8;
        result_num = result_num | (target_num & 0xff);
        target_num = target_num >> 8;
    }
    /* result_num = result_num | ((target_num & 0x000000ff)<<24);
    result_num = result_num | ((target_num & 0x0000ff00)<<8);
    result_num = result_num | ((target_num & 0x00ff0000)>>8);
    result_num = result_num | ((target_num & 0xff000000)>>24);
    */
    printf("convert %x to %x\n",ori_target_num,result_num);
    return result_num;
}

/*
    param:1.target uint64_t num
    function: convert Big Endian or Little Endian of a uint64_t num
    return: the converted num or the original num
 */
uint64_t convert_uint64_t(uint64_t target_num){
    if(need_convert==0){
        return target_num;
    }
    uint64_t ori_target_num = target_num;
    uint64_t result_num;
    result_num = 0;
    for(int i=0;i<8;i++){
        result_num = result_num << 8;
        result_num = result_num | (target_num & 0xff);
        target_num = target_num >> 8;
    }
    printf("convert %lx to %lx\n",ori_target_num,result_num);
    return result_num;
}

/*
    param:1.target file's name
    function: map the target fat binary file into memory 
    return : the address of the mapped file
*/
uint8_t* map_file_in_mem(char* file_name){
    FILE* tmp_file = fopen(file_name,"rb");
    fseek(tmp_file,0,SEEK_END);
    uint64_t file_size = ftell(tmp_file);
    fseek(tmp_file,0,0);
    fclose(tmp_file);
    int target_file = open(file_name,O_RDONLY);
    if(target_file<0){
        printf("open target_file failed \n");
    }
    uint8_t* file_pointer;
    file_pointer = (uint8_t*)mmap(NULL,file_size,PROT_READ,MAP_PRIVATE,target_file,0);
    return file_pointer;
}

/*
   param: 1.the start address of the fatBinary; 2.the offset of the armv8_64 arch; 3. the size of the armv8_64 file; 4.Is the fatBinary a 64bit file
   function: dump the armv8_64 file by the offset and size;
   return: void
*/
void dump_file(uint8_t* file_p, uint64_t offset, uint64_t size, uint32_t is_64){
    FILE* tmp_file;
    if(is_64==1){
        tmp_file = fopen("result_file_64","wb");
    }
    else{
        tmp_file = fopen("result_file","wb");
    }
    file_p = file_p + offset;
    for(uint64_t i=0;i<size;i++){
        fputc(*(file_p+i),tmp_file);
    }
    fclose(tmp_file);
}

/*
    param:1.the start address of the fatBinary
    function: read the fat header and check each arch to get the offset and size of the armv8_64
    return void
*/
void extract_file_from_fat(uint8_t* file_p){
    printf("start to extract file from fat\n");
    fat_header* fat_h_p = (fat_header*)file_p;
    uint32_t target_arch_num;
     target_arch_num = convert_uint32_t(fat_h_p->nfat_arch);
    printf("target_arch_num : %x\n",target_arch_num);
    fat_arch* target_arch;
     
    for(int i=0;i<target_arch_num;i++){
        target_arch = (fat_arch*)(file_p+sizeof(fat_header)+sizeof(fat_arch)*i); 
        uint32_t tmp_cputype;
        uint32_t tmp_subcputype;
        tmp_cputype = convert_uint32_t(target_arch->cputype);
        tmp_subcputype = convert_uint32_t(target_arch->cpusubtype);
        if(tmp_cputype == CPU_TYPE_ARM64){
            if(tmp_subcputype == CPU_SUBTYPE_ARM64_V8){
                uint64_t tmp_offset = (uint64_t) convert_uint32_t(target_arch->offset);
                uint64_t tmp_size = (uint64_t) convert_uint32_t(target_arch->size);
                dump_file(file_p,tmp_offset, tmp_size,0);
            }
        }
        /*
           for test
         */
       /*  if(tmp_cputype == CPU_TYPE_X86){
            if(tmp_subcputype == CPU_SUBTYPE_X86_ALL){
                uint64_t tmp_offset = (uint64_t) convert_uint32_t(target_arch->offset);
                uint64_t tmp_size = (uint64_t) convert_uint32_t(target_arch->size);
                printf("tmp_offset : %lx \n",tmp_offset);
                printf("tmp_size : %lx \n",tmp_size);
                dump_file(file_p,tmp_offset, tmp_size,0);
            }
        }*/
        /*
           for test end
         */
    }
    
}

/*
    param:1.the start address of the fatBinary
    function: read the fat header and check each arch to get the offset and size of the armv8_64 , the fat binary is fat_64;
    return void
*/
void extract_file_from_fat_64(uint8_t* file_p){
    printf("start to extract file from fat_64");
    fat_header* fat_h_p = (fat_header*)file_p;
    uint32_t target_arch_num = convert_uint32_t(fat_h_p->nfat_arch);
    printf("target_arch_num : %x",target_arch_num);
    fat_arch_64* target_arch;
     
    for(int i=0;i<target_arch_num;i++){
        target_arch = (fat_arch_64*)(file_p+sizeof(fat_header)+sizeof(fat_arch_64)*i);
        if(convert_uint32_t(target_arch->cputype) == CPU_TYPE_ARM64){
            if(convert_uint32_t(target_arch->cpusubtype) == CPU_SUBTYPE_ARM64_V8){
                dump_file(file_p,(uint64_t)convert_uint64_t(target_arch->offset), (uint64_t)convert_uint64_t(target_arch->size),1);
            }
        }
    }
}

int main(){
    char* target_file_name = "target_file";

    uint8_t* file_pointer;
    file_pointer = map_file_in_mem(target_file_name);
    
    fat_header* tmp_fat_header_point;
    tmp_fat_header_point = (fat_header*)file_pointer;

    if(tmp_fat_header_point->magic == FAT_MAGIC){
        printf("start to extract fat");
        need_convert = 0;
        extract_file_from_fat(file_pointer);
    }
    if(tmp_fat_header_point->magic == FAT_MAGIC_64){
            printf("start to extract fat64");
            need_convert = 0;
            extract_file_from_fat_64(file_pointer);
    }
    if(tmp_fat_header_point->magic == FAT_CIGAM){
        printf("start to extract fat from different version");
        need_convert = 1;
        extract_file_from_fat(file_pointer);
    }
    if(tmp_fat_header_point->magic == FAT_CIGAM_64){
        printf("start to extract fat from different version");
        need_convert = 1;
        extract_file_from_fat_64(file_pointer);
    }
}

