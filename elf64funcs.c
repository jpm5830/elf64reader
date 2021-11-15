//
// Created by jpm on 10/31/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "elf64funcs.h"

// GLOBAL VARIABLES
Elf64_Shdr section[SH_CAPACITY];
char *section_names[SH_CAPACITY];
FILE *fp = NULL;

extern const char *progname;
static Elf64_Ehdr file_hdr; // elf file header struct
static long sh_name_str_tab_off; // section header name string table offset

static int set_sh_name_str_tab_off() {
    unsigned long ul;
    long shstrtab_off;
    Elf64_Shdr shdr;

    // Seek to offset of section header string table
    ul = file_hdr.e_shoff + file_hdr.e_shentsize * file_hdr.e_shstrndx;
    if (ul & 0x8000000000000000) { // unsigned val has sign bit set
        fprintf(stderr, "ERROR: Bad conversion of unsigned long to long detected in function '%s'\n",
                __FUNCTION__);
        return EXIT_FAILURE;
    }
    shstrtab_off = (long) ul; // safe cast

    if (fseek(fp, shstrtab_off, SEEK_SET) != 0) {
        fprintf(stderr, "ERROR: Seeking to start of shstrtab_off (%lx) in function '%s'\n",
                shstrtab_off, __FUNCTION__);
        return EXIT_FAILURE;
    }

    // Read in the .shstrtab section info
    if (fread(&shdr, 1, file_hdr.e_shentsize, fp) != file_hdr.e_shentsize) {
        fprintf(stderr, "ERROR: Bad read of section header index %d (.shstrtab) in function '%s'\n",
                file_hdr.e_shstrndx, __FUNCTION__);
        return EXIT_FAILURE;
    }

    if (shdr.sh_offset & 0x8000000000000000) { // unsigned val has sign bit set
        fprintf(stderr, "ERROR: Bad conversion of unsigned long to long detected in function '%s'\n",
                __FUNCTION__);
        return EXIT_FAILURE;
    }

    sh_name_str_tab_off = (long) shdr.sh_offset; // safe cast
    return EXIT_SUCCESS;
}

// caller needs to free returned pointer
static char *get_section_name(int n) {
    int name_len = 0;
    long name_off;
    char *cp = NULL;

    name_off = sh_name_str_tab_off + section[n].sh_name;
    fseek(fp, name_off, SEEK_SET);
    while (fgetc(fp) != '\0')
        name_len++;
    fseek(fp, name_off, SEEK_SET);
    name_len++; // increase space needed for '\0'
    cp = (char *) malloc(name_len);
    fgets(cp, name_len, fp);
    return cp;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "DanglingPointer"

static void set_section_names() {
    char *cp = NULL;

    for (int n = 0; n < file_hdr.e_shnum; n++) {
        if ((cp = get_section_name(n))) {
            section_names[n] = cp; // save it to global array
        }
    }
}

#pragma clang diagnostic pop

int get_section_num_by_name(const char *sn) {
    if (!sn) // index 0 has no name
        return 0;
    for (int i = 0; i < file_hdr.e_shnum; i++) {
        if (!strncmp(sn, section_names[i], strlen(sn)))
            return i;
    }
    return -1; // on failure to find sn
}

int print_file_header() {

    printf("\nELF FILE HEADER\n");
    printf("---------------\n");

    printf("Magic:  ");
    for (int i = 0; i < EI_NIDENT; i++)
        printf(" %02x", file_hdr.e_ident[i]);
    printf("\n");

    switch (file_hdr.e_ident[EI_CLASS]) {
        case ELFCLASS64:
            printf("%-40s: %s\n", "Class", "ELF64");
            break;
        case ELFCLASS32:
            printf("%-40s: %s\n", "Class", "ELF32");
            break;
        default:
            fprintf(stderr, "%-40s: %s\n", "Class", "Unknown value");
            return EXIT_FAILURE;
    }

    switch (file_hdr.e_ident[EI_DATA]) {
        case ELFDATA2LSB:
            printf("%-40s: %s\n", "Data", "2's compliment LSB (little endian)");
            break;
        case ELFDATA2MSB:
            printf("%-40s: %s\n", "Data", "2's compliment MSB (big endian)");
            break;
        default:
            fprintf(stderr, "Can't identify endian type for e_ident[EI_DATA]\n");
            return EXIT_FAILURE;
    }

    printf("%-40s: %d\n", "Object File Format Version", file_hdr.e_ident[EI_VERSION]);

    printf("%-40s: ", "OS/ABI");
    switch (file_hdr.e_ident[EI_OSABI]) {
        case ELFOSABI_SYSV:
            printf("Unix - System V\n");
            break;
        case ELFOSABI_HPUX:
            printf("Using the HPUX operating system\n");
            break;
        case ELFOSABI_LINUX:
            printf("Object uses GNU/Linux extensions\n");
            break;
        case ELFOSABI_STANDALONE:
            printf("Standalone (embedded) application\n");
            break;
        default:
            printf("Undecoded value (%d) for e_ident[EI_OSABI]\n", file_hdr.e_ident[EI_OSABI]);
    }

    printf("%-40s: %d\n", "ABI Version", file_hdr.e_ident[EI_ABIVERSION]);

    printf("%-40s: ", "Type");
    switch (file_hdr.e_type) {
        case ET_NONE: //No file type
            printf("NONE (no type info)\n");
            break;
        case ET_REL:
            printf("REL (relocatable object file)\n");
            break;
        case ET_EXEC:
            printf("EXEC (executable file)\n");
            break;
        case ET_DYN:
            printf("DYN (shared object file)\n");
            break;
        case ET_CORE:
            printf("CORE (core file)\n");
            break;
        case ET_LOOS: // Environment-specific use
        case ET_HIOS:
            printf("Environment-specific use: %d\n", file_hdr.e_type);
            break;
        case ET_LOPROC: //Processor-specific use
        case ET_HIPROC:
            printf("Processor-specific use: %d\n", file_hdr.e_type);
            break;
        default:
            printf("Unknown value for e_type: %d", file_hdr.e_type);
    }

    if (file_hdr.e_machine == EM_X86_64)
        printf("%-40s: %s (%d)\n", "Machine", "AMD x86-64 architecture", file_hdr.e_machine);
    else
        printf("%-40s: %d\n", "Machine", file_hdr.e_machine);

    printf("%-40s: %d\n", "Object File Format Version", file_hdr.e_version);
    printf("%-40s: %p\n", "Entry point address", (void *) file_hdr.e_entry);
    printf("%-40s: %lu (bytes into file)\n", "Start of program headers", file_hdr.e_phoff);
    printf("%-40s: %lu (bytes into file)\n", "Start of section headers", file_hdr.e_shoff);
    printf("%-40s: %u\n", "Processor specific flags", file_hdr.e_flags);
    printf("%-40s: %u (bytes)\n", "Size of file header", file_hdr.e_ehsize);
    printf("%-40s: %u (bytes)\n", "Size of program header entry", file_hdr.e_phentsize);
    printf("%-40s: %u\n", "Number of program header entries", file_hdr.e_phnum);
    printf("%-40s: %u (bytes)\n", "Size of section header entry", file_hdr.e_shentsize);
    printf("%-40s: %u\n", "Number of section header entries", file_hdr.e_shnum);
    printf("%-40s: %u\n", "Section name string table index", file_hdr.e_shstrndx);
    return (EXIT_SUCCESS);
}

static int sh_read(int n, Elf64_Shdr *shp) {
    long start_off;
    unsigned long ul;

    // n should be between 0 and number of sections - 1, check it
    if ((n < 0) || (n >= file_hdr.e_shnum)) {
        fprintf(stderr, "ERROR: Invalid section index (%d) parameter in function '%s'.\n",
                n, __FUNCTION__);
        return EXIT_FAILURE;
    }

    ul = file_hdr.e_shoff + file_hdr.e_shentsize * n;
    if (ul & 0x8000000000000000) { // unsigned val has sign bit set
        fprintf(stderr, "ERROR: Bad conversion of unsigned long to long detected in function '%s'\n",
                __FUNCTION__);
        return EXIT_FAILURE;
    }
    start_off = (long) ul; // safe cast

    // Read and store section n header info
    // Note narrowing conversion from unsigned long to long for param start_off
    if (fseek(fp, start_off, SEEK_SET) != 0) {
        fprintf(stderr, "ERROR: Seeking to start of section %d, (%lx) in function '%s'.\n",
                n, start_off, __FUNCTION__);
        return EXIT_FAILURE;
    }
    if (fread(shp, 1, file_hdr.e_shentsize, fp) != file_hdr.e_shentsize) {
        fprintf(stderr, "ERROR: Bad read of section header %d in function '%s'.\n",
                n, __FUNCTION__);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int sh_readall() {
    int rc;
    for (int n = 0; n < file_hdr.e_shnum; n++) {
        if ((rc = sh_read(n, &(section[n]))) == EXIT_FAILURE) {
            return rc;
        }
    }
    return EXIT_SUCCESS;
}

static void sh_print(int n) {
    char *hdr = "\nSection Headers:\n"
                "[Nr] Name              Type             Address           Offset\n"
                "     Size              EntSize          Flags  Link  Info  Align";
    static int section_header_printed = 0;
    char *section_name = NULL;

    // Only print the header (hdr) once for all section headers
    if (!section_header_printed) {
        printf("%s\n", hdr);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
        section_header_printed++;
#pragma clang diagnostic pop
    }

    // todo: Error check the function below
    if ((section_name = get_section_name(n)) != NULL) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "DanglingPointer"
        printf("[%-2d] %-16s  ", n, section_name); // section number and name
        free(section_name);
#pragma clang diagnostic pop

        section_name = NULL;
    } else
        fprintf(stderr, "WARNING: section name error.\n");

    // Section type
    switch (section[n].sh_type) {
        case SHT_NULL   :
            printf("%-16s ", "NULL");
            break;
        case SHT_PROGBITS   :
            printf("%-16s ", "PROGBITS");
            break;
        case SHT_SYMTAB :
            printf("%-16s ", "SYMTAB");
            break;
        case SHT_STRTAB :
            printf("%-16s ", "STRTAB");
            break;
        case SHT_RELA   :
            printf("%-16s ", "RELA");
            break;
        case SHT_HASH   :
            printf("%-16s ", "HASH");
            break;
        case SHT_DYNAMIC    :
            printf("%-16s ", "DYNAMIC");
            break;
        case SHT_NOTE   :
            printf("%-16s ", "NOTE");
            break;
        case SHT_NOBITS :
            printf("%-16s ", "NOBITS");
            break;
        case SHT_REL        :
            printf("%-16s ", "REL");
            break;
        case SHT_SHLIB  :
            printf("%-16s ", "SHLIB");
            break;
        case SHT_DYNSYM :
            printf("%-16s ", "DYNSYM");
            break;
        case SHT_INIT_ARRAY :
            printf("%-16s ", "INIT_ARRAY");
            break;
        case SHT_FINI_ARRAY :
            printf("%-16s ", "FINI_ARRAY");
            break;
        case SHT_PREINIT_ARRAY:
            printf("%-16s ", "PREINIT_ARRAY");
            break;
        case SHT_GROUP  :
            printf("%-16s ", "GROUP");
            break;
        case SHT_SYMTAB_SHNDX:
            printf("%-16s ", "SYMTAB_SHNDX");
            break;
        case SHT_NUM    :
            printf("%-16s ", "NUM");
            break;
        case SHT_LOOS   :
            printf("%-16s ", "LOOS");
            break;
        case SHT_GNU_ATTRIBUTES:
            printf("%-16s ", "GNU_ATTRIBUTES");
            break;
        case SHT_GNU_HASH   :
            printf("%-16s ", "GNU_HASH");
            break;
        case SHT_GNU_LIBLIST    :
            printf("%-16s ", "GNU_LIBLIST");
            break;
        case SHT_CHECKSUM   :
            printf("%-16s ", "CHECKSUM");
            break;
        case SHT_LOSUNW :
            printf("%-16s ", "LOSUNW");
            break;
        case SHT_SUNW_COMDAT:
            printf("%-16s ", "SUNW_COMDAT");
            break;
        case SHT_SUNW_syminfo:
            printf("%-16s ", "SUNW_syminfo");
            break;
        case SHT_GNU_verdef :
            printf("%-16s ", "GNU_verdef");
            break;
        case SHT_GNU_verneed    :
            printf("%-16s ", "GNU_verneed");
            break;
        case SHT_GNU_versym :
            printf("%-16s ", "GNU_versym");
            break;
        case SHT_LOPROC :
            printf("%-16s ", "LOPROC");
            break;
        case SHT_HIPROC :
            printf("%-16s ", "HIPROC");
            break;
        case SHT_LOUSER :
            printf("%-16s ", "LOUSER");
            break;
        case SHT_HIUSER:
            printf("%-16s ", "HIUSER");
            break;
        default:
            printf("%-16s ", "NOTFOUND");
    }
    // Address
    printf("%016lx  ", section[n].sh_addr);
    // Offset
    printf("%016lx\n", section[n].sh_offset);
    // Size
    printf("     %016lx  ", section[n].sh_size);
    // Entry size
    printf("%016lx  ", section[n].sh_entsize);
    // Flags
    printf("%4lx  ", section[n].sh_flags);
    // Link
    printf("%4u  ", section[n].sh_link);
    // Info
    printf("%4u ", section[n].sh_info);
    // Alignment
    printf("%16lu\n", section[n].sh_addralign);
}

void print_section_headers() {
    for (int n = 0; n < file_hdr.e_shnum; n++)
        sh_print(n);
}

void print_section_names() {
    for (int n = 0; n < file_hdr.e_shnum; n++) {
        printf("[%2d]: %s\n", n, section_names[n]);
    }
}

void elf64funcs_close() {
    for (int i = 0; i < file_hdr.e_shnum; i++) {
        free(section_names[i]);
        section_names[i] = NULL;
    }
}

int elf64funcs_init(const char *fname) {
    const char magic[4] = "\177ELF";
    size_t cr; // chars read

    assert(fname != NULL);

    // Attempt to open the file
    if ((fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "Couldn't open file '%s'\n", fname);
        exit(EXIT_FAILURE);
    }
    // Attempt to read in the file header
    rewind(fp);
    if ((cr = fread(&file_hdr, 1, sizeof(Elf64_Ehdr), fp)) != sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "Error reading elf header: %lu chars read, %lu chars expected\n",
                cr, sizeof(Elf64_Ehdr));
        return EXIT_FAILURE;
    }
    // Check that this is a 64 bit ELF file
    if ((memcmp(&file_hdr, magic, sizeof(magic)) != 0) || (file_hdr.e_ident[EI_CLASS] != ELFCLASS64)) {
        fprintf(stderr, "File is not a 64 bit ELF file\n");
        return EXIT_FAILURE;
    }
    // Check there is enough capacity in section to hold all section headers
    if (file_hdr.e_shnum > SH_CAPACITY) {
        fprintf(stderr, "Not enough section header capacity for this file.\n");
        fprintf(stderr, "Needed: %u, Available: %u\n", file_hdr.e_shnum, SH_CAPACITY);
        return EXIT_FAILURE;
    }
    sh_readall();
    // Set file offset to section header name string table
    set_sh_name_str_tab_off();
    set_section_names();
    return EXIT_SUCCESS;
}
