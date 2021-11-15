//
// Created by jpm on 10/31/21.
// Function prefixes: sh_ (section header), fh_ (file header), ph_ (program header)
//

#ifndef ELF64FUNCS_H
#define ELF64FUNCS_H

#include <elf.h>

#define SH_CAPACITY 80 // max number of section headers allowed

// GLOBAL VARIABLES
extern Elf64_Shdr section[SH_CAPACITY];
extern char *section_names[SH_CAPACITY];

// FUNCTION DECLARATIONS
int elf64funcs_init(const char *fname); // call this function before using any others

void elf64funcs_close(); // call this function last. Cleans up any dynamically allocated  memory

int print_file_header();

void print_section_headers();

void print_section_names();

int get_section_num_by_name(const char *sn);

#endif //ELF64FUNCS_H
