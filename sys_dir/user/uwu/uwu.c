#include <stdio.h>
#include <stdlib.h>
#include <ktype.h>

int main(void){
    int32_t hexa;
    FILE *elffile ;
    elffile = fopen("potatus.txt","rb");
    fgets("%x",0,elffile);
    printf("%x",hexa);
    return 0;
}