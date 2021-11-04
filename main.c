#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "elf64funcs.h"

#define SH_CAPACITY 80 // max number of section headers allowed

extern FILE *fp;

// GLOBALS
const char *progname = "elf64reader";


// FUNCTIONS

static void usage() {
    const char *help_menu = {
            "   -H\tHelp Menu\n"
            "   -a\tEquivalent to: -h -S\n"
            "   -h\tPrint elf file header\n"
            "   -S\tPrint all section headers"
    };
    const char *hdr = {
            " Display information about the contents of ELF format files\n"
            " Options are:"
    };

    fprintf(stderr, "Usage: %s <options> elf-file\n", progname);
    fprintf(stderr, "%s\n", hdr);
    fprintf(stderr, "%s\n", help_menu);
}

int main(int argc, char *argv[]) {
    int oc; // option char
    int status = EXIT_SUCCESS;
    // FLAGS (0 is OFF, any other value is ON)
    int help_flag = 0; // print help
    int print_sh_flag = 0; // print section header flag
    int print_fh_flag = 0; // print file header flag

    if (argc < 3) {
        usage();
        exit(EXIT_FAILURE);
    }

    while ((oc = getopt(argc, argv, ":HahS")) != -1) {
        switch (oc) {
            case 'H': // help
                help_flag = 1;
                break;
            case 'a': // all
                print_fh_flag = 1;
                print_sh_flag = 1;
                break;
            case 'h': // print elf header
                print_fh_flag = 1;
                break;
            case 'S': // print all section headers
                print_sh_flag = 1;
                break;
            default:
                fprintf(stderr, "Invalid option -%c: ignored\n", optopt);
        }
    }

    if (help_flag) {
        usage();
        exit(EXIT_SUCCESS);
    }
    printf("\n%s program pid is %d.\n", progname, getpid());

    if ((status = fh_read(argv[optind])) == EXIT_FAILURE)
        goto done;

    if ((status = sh_readall()) == EXIT_FAILURE)
        goto done;

    if (print_fh_flag)
        fh_print();
    if (print_sh_flag)
        sh_printall();

    done:
    if (fp)
        fclose(fp);
    return status;
}




