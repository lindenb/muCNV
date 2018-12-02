//
//  vcf_to_idx.cpp
//  muCNV
//
//  Created by Goo Jun on 11/22/18.
//  Copyright © 2018 Goo Jun. All rights reserved.
//

#include <stdio.h>
// TCLAP headers
#include "tclap/CmdLine.h"
#include "tclap/Arg.h"

#include "muCNV.h"

int main_vcf_to_int(int argc, char** argv)
{
    string vcf_file;
    string interval_file;
    bool bPrint;
    std::vector<sv> vec_sv;
    std::vector<breakpoint> vec_bp;
    
    try
    {
        TCLAP::CmdLine cmd("Command description message", ' ', "0.06");
        
        TCLAP::ValueArg<string> argVcf("v","vcf","VCF file containing candidate SVs",true,"","string");
        TCLAP::ValueArg<string> argInterval("i","interVal", "Binary interval file containing candidate SVs", false, "", "string");
        TCLAP::SwitchArg bPrint("p","print", "Print out SV variants", cmd, false);

        cmd.add(argVcf);
        cmd.add(argInterval);
        
        cmd.parse(argc, argv);

        vcf_file = argVcf.getValue();
        interval_file = argInterval.getValue();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        abort();
    }
    // TODO: error checking
    read_svs_from_vcf(vcf_file, vec_bp, vec_sv);
    if (interval_file != "")
    {
        write_interval(interval_file, vec_sv);
    }
    if (bPrint)
    {
        for(int i=0; i<(int)vec_sv.size(); ++i)
        {
            vec_sv[i].print();
            printf("\n");
        }

    }
    return 0;
}
