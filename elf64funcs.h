//
// Created by jpm on 10/31/21.
// Function prefixes: sh_ (section header), fh_ (file header), ph_ (program header)
//

#ifndef ELF64FUNCS_H
#define ELF64FUNCS_H

#include <elf.h>

// FUNCTION DECLARATIONS
int fh_read(const char *fname);

int fh_print();

int sh_read(int n, Elf64_Shdr *shp);

int sh_readall();

void sh_print(int n);

void sh_printall();

int set_sh_name_str_tab_off();

#endif //ELF64FUNCS_H
