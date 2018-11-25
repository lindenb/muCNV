/*
 *   Author: Goo Jun (goo.jun@uth.tmc.edu)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <map>

#include "muCNV.h"
#include "gc_content.h"
#include "bam_cram.h"

// Main functions
int main_pileup(int, char**);
int main_merge_pileup(int, char**);
int main_genotype(int, char**);
int main_print_pileup(int, char**);
int main_vcf_to_int(int, char**);
int main_gcidx(int, char**);
int main_filter(int, char**);

int main(int argc, char** argv)
{
    int ret = 0; // exit code
    
	std::cerr << "muCNV 0.9.1, large-scale multi-sample SV genotyper" << std::endl;
	std::cerr << "(c) 2018 Goo Jun" << std::endl << std::endl;
	std::cerr.setf(std::ios::showpoint);
    
    srand((unsigned int)time(NULL));

    // muCNV run modes

    // muCNV pileup
    //      single-sample pileup
    if (strcmp(argv[1], "pileup") == 0)
        ret = main_pileup(argc-1, argv+1);
    
    // muCNV merge
    //      merge single-sample pileups into multi-sample mpileups
    else if (strcmp(argv[1], "merge"))
        ret = main_merge_pileup(argc-1, argv+1);

    // muCNV genotype
    //      variant call (uses: read multiple mpileup or pileup, call variants, weights(?), VCF out)
    else if (strcmp(argv[1], "genotype"))
        ret = main_genotype(argc-1, argv+1);

    // muCNV print
    //      read single-sample or multi-sample pileup to print out
    else if (strcmp(argv[1], "print"))
        ret = main_print_pileup(argc-1, argv+1);

    // muCNV vcf2int
    //      read VCF to make binary interval lists
    else if (strcmp(argv[1], "vcf2int"))
        ret = main_vcf_to_int(argc-1, argv+1);

    // muCNV gcidx
    //      Generate GC content table from reference FASTA
    else if (strcmp(argv[1], "gcidx"))
        ret = main_gcidx(argc-1, argv+1);
    
    // muCNV filter
    //      Merge / filter SVs from VCF
    else if (strcmp(argv[1], "gcidx"))
        ret = main_filter(argc-1, argv+1);

    return ret;
}
