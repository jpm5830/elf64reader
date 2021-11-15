//
// Created by jpm on 10/31/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "elf64funcs.h"

extern FILE *fp;

// GLOBALS
const char *progname = "elf64reader";


// FUNCTIONS

static void usage() {
    const char *help_menu = {
            "   -H\tHelp Menu\n"
            "   -a\tEquivalent to: -h -S\n"
            "   -h\tPrint elf file header\n"
            "   -S\tPrint all section headers\n"
            "   -n\tPrint all section names"
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
    int print_section_names_flag = 0;

    if (argc < 3) {
        usage();
        exit(EXIT_FAILURE);
    }

    while ((oc = getopt(argc, argv, ":HahSn")) != -1) {
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
            case 'n': // print all section names
                print_section_names_flag = 1;
                break;
            default:
                fprintf(stderr, "Invalid option -%c: ignored\n", optopt);
        }
    }

    if (help_flag) {
        usage();
        exit(EXIT_SUCCESS);
    }
    printf("\n%s program pid is %d.\n\n", progname, getpid());

    if ((status = elf64funcs_init(argv[optind])) == EXIT_FAILURE)
        goto done;

    if (print_fh_flag)
        print_file_header();
    if (print_sh_flag)
        print_section_headers();
    if (print_section_names_flag)
        print_section_names();

    done:
    if (fp)
        fclose(fp);
    elf64funcs_close();
    return status;
}
