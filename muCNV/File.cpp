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
#include "muCNV.h"
#include <stdlib.h>

void read_vcf_list(string &index_file, vector<string> &vcf_files)
{
	ifstream inFile(index_file.c_str(), ios::in);
	
	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		if (!ln.empty())
		{
			vcf_files.push_back(ln);
		}
	}
	cerr<< vcf_files.size() << " VCF files exist in the index file." << endl;
}
		
	
void read_index(string index_file, vector<string> &sample_ids, vector<string> &vcf_files, vector<string> &bam_files, vector<double> &avg_depths)
{
	ifstream inFile(index_file.c_str(), ios::in);
	
	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		if (!ln.empty())
		{
			vector<string> tokens;
			split(ln.c_str(), " \t\n", tokens);
			
			if (tokens[0].empty())
			{
				cerr << "Error loading index: empty sample ID"<< endl;
				exit(1);
			}
			else if (tokens[1].empty())
			{
				cerr << "Error loading index: cannot find vcf files" << endl;
			}
			else if (tokens[2].empty())
			{
				cerr << "Error loading index: cannot find BAM/CRAM files" << endl;
			}
			else if (tokens[3].empty())
			{
				cerr << "Error loading index: cannot find average depth info" << endl;
			}
			sample_ids.push_back(tokens[0]);
			vcf_files.push_back(tokens[1]);
			bam_files.push_back(tokens[2]);
			avg_depths.push_back(atof(tokens[3].c_str()));
		}
	}
	inFile.close();
	
}


int invcfs::initialize(vector<string> &vcf_files, vector<string> &sample_ids, vector<double> &avg_depths, vector<double> &avg_isizes)
{
	//vector<string> lns (vcf_files.size(), "");
	int n_vcf = (int)vcf_files.size();
	
	for(int i=0;i<n_vcf; ++i)
	{
		//	cerr << "sample ID " << sample_ids[i] << endl;
		//		cerr << "opening " << vcf_files[i] << endl;
		ifstream *f = new ifstream(vcf_files[i].c_str(), ios::in);
		vfs.push_back(f);
	}
	
	start_num.resize(n_vcf);
	num_id.resize(n_vcf);
	
	for(int i=0;i<n_vcf;++i)
	{
		if (!vfs[i]->good())
		{
			cerr<< "Error initializing VCF files\n" << endl;
			return -1;
		}
		start_num[i] = 0;
		num_id[i] = 0;
	}

	
	for(int i=0;i<n_vcf;++i)
	{
		if (i>0)
		{
			start_num[i] = start_num[i-1] + num_id[i-1];
		}
		bool flag = true;
		while(flag)
		{
			string ln;
			getline(*vfs[i],ln);
			if (ln.empty())
			{
				// Error! Empty line in the header
			}
			else if (ln[0] == '#' && ln[1] == 'C' && ln[2] == 'H' && ln[3] == 'R')
			{
				// read sample ids
				vector<string> tokens;
				split(ln.c_str(), " \t\n", tokens);
				for(int j=9; j<tokens.size(); ++j)
				{
					num_id[i]++;
					sample_ids.push_back(tokens[j]);
				}
			}
			else if (ln[0] == '#' && ln[1] == '#')
			{
				// Skip these header lines
			}
			else if (ln[0] == '0' && (ln[1] == '\t' || ln[1] == ' '))
			{
				// Read avg depth
				flag = false;
				vector<string> tokens;
				split(ln.c_str(), " \t\n", tokens);
				for(int j=9;j<tokens.size();++j)
				{
					vector<string> fields;
					split(tokens[j].c_str(), ":", fields);

					avg_depths.push_back(atof(fields[0].c_str()));
					avg_isizes.push_back(atof(fields[1].c_str()));
				}
			}
		}
		if (sample_ids.size() != avg_depths.size() || avg_depths.size() != avg_isizes.size())
		{
			cerr << "Error: number of fields for avegerage depth does not match" << endl;
			cerr << "sample ids has " << sample_ids.size() << " and average depths has " << avg_depths.size() << endl;
			return -1;
		}
	}
	cerr << "Input VCF files initialized" <<endl;
	return 0;
}


void invcfs::parse_sv(vector<string> &tokens, sv& interval)
{
	int chr;

	if (tokens[0].substr(0,3) == "chr" || tokens[0].substr(0,3) == "Chr" )
	{
		tokens[0] = tokens[0].substr(3,2);
	}
	if (tokens[0] == "X")
	{
		chr = 23;
	}
	else if (tokens[0] == "Y")
	{
		chr = 24;
	}
	else if (tokens[0] == "M" || tokens[0] == "MT")
	{
		chr = 25;
	}
	else
	{
		try
		{
			chr = atoi(tokens[0].c_str());
		}
		catch(int e)
		{
			chr = 0;
			
		}
	}
	interval.chr = tokens[0]; // Dec 1, 2017
	interval.chrnum = chr;
	interval.pos = atoi(tokens[1].c_str());

	string info = tokens[7];

	vector<string> infotokens;

	split(info.c_str(), ";", infotokens);
	for(int j=0;j<(int)infotokens.size();++j)
	{
		vector<string> infofields;
		split(infotokens[j].c_str(), "=", infofields);
		if (infofields.size()>1)
		{
			if (infofields[0] == "END")
			{
				interval.end = atoi(infofields[1].c_str());
			}

			else if (infofields[0] == "SVTYPE")
			{
				interval.svtype = infofields[1];
			}
		}
		
	} // if chr >= 1 && chr <= 22
}

double invcfs::get_second_value(string &t)
{
	if (t==".") return 0;
	
	int p=0;
	while(t[p++] != ',');
	return abs(atof(t.substr(p,string::npos).c_str()));
}

int invcfs::read_interval_multi(sv& interval, vector<double> &dp, vector<double> &isz_cnv_pos,
								vector<double> &isz_cnv_neg, vector<double> &isz_inv_pos, vector<double> &isz_inv_neg)
{
	int idx = 0;
	
	for(int i=0;i<(int)vfs.size();++i)
	{
		if (!vfs[i]->good())
			return -1;
	}
	
	for(int i=0;i<(int)vfs.size();++i)
	{
		bool flag = true;
		while(flag)
		{
			string ln;
			getline(*vfs[i],ln);
			if (ln.empty())
			{
				// Error! Empty line in the header
				if (i>0)
				{
					cerr << "Error reading VCF files" << endl;
					exit(1);
				}
				return -1;
			}
			else if (ln[0] == '#')
			{
				// Skip these header lines
				// Should never happen because headers are already processed in initialize()
			}
			else
			{
				
				// Read per-sample depth, insert size info
				flag = false;
				vector<string> tokens;
				split(ln.c_str(), " \t\n", tokens);
				
				// Read interval information
				if (i==0)
				{
					// Parse SV info only from the first VCF
					parse_sv(tokens, interval);
				}

				for(int j=9;j<tokens.size();++j)
				{
					vector<string> fields;
					split(tokens[j].c_str(), ":", fields);
					
					// GC corrected depth
					dp[idx] = atof(fields[1].c_str());
					
					isz_cnv_pos[idx] = get_second_value(fields[2]);
					isz_cnv_neg[idx] = get_second_value(fields[3]);
					isz_inv_pos[idx] = get_second_value(fields[4]);
					isz_inv_neg[idx] = get_second_value(fields[5]);
					idx++;

				}
			}
		}
	}

	return 0;
}

int invcfs::read_interval(sv& interval, vector<double> &X)
{
	vector<string> lns (vfs.size(), "");

	for(int i=0;i<(int)vfs.size();++i)
	{
		if (!vfs[i]->good())
			return -1;
	}
	bool flag=false;
	int cnt = 0;
	for(int i=0;i<(int)vfs.size();++i)
	{
		getline(*vfs[i],lns[i]);

		if (lns[i].empty() || lns[i][0] == '#')
		{
			flag = true;
		}
		cnt++;
	}
	if (flag)
	{
		if (cnt>0)
			return -1;
		else
			return 1;
	}

	int chr;
	vector<string> tokens;
	split(lns[0].c_str(), " \t\n", tokens);
	// Let's add error handling later
	if (tokens[0].substr(0,3) == "chr" || tokens[0].substr(0,3) == "Chr" )
	{
		tokens[0] = tokens[0].substr(3,2);
	}
	if (tokens[0] == "X")
	{
		chr = 23;
	}
	else if (tokens[0] == "Y")
	{
		chr = 24;
	}
	else if (tokens[0] == "M" || tokens[0] == "MT")
	{
		chr = 25;
	}
	else
	{
		try
		{
			chr = atoi(tokens[0].c_str());
		}
		catch(int e)
		{
			chr = 0;
			
		}
	}
	interval.chr = tokens[0]; // Dec 1, 2017
	interval.chrnum = chr;
	interval.pos = atoi(tokens[1].c_str());
//	interval.ci_pos.first = -1;
//	interval.ci_pos.second = -1;
//	interval.ci_end.first = -1;
//	interval.ci_end.second = -1;
	string info = tokens[7];
	
	vector<string> infotokens;
	
	split(info.c_str(), ";", infotokens);
	for(int j=0;j<(int)infotokens.size();++j)
	{
		vector<string> infofields;
		split(infotokens[j].c_str(), "=", infofields);
		if (infofields.size()>1)
		{
			if (infofields[0] == "END")
			{
				interval.end = atoi(infofields[1].c_str());
			}
			/*
			else if (infofields[0] == "CIPOS")
			{
				vector<string> ci;
				split(infofields[1].c_str(), ",", ci);
				interval.ci_pos.first = atoi(ci[0].c_str());
				interval.ci_pos.second = atoi(ci[1].c_str());
			}
			else if (infofields[0] == "CIEND")
			{
				vector<string> ci;
				split(infofields[1].c_str(), ",", ci);
				interval.ci_end.first = atoi(ci[0].c_str());
				interval.ci_end.second = atoi(ci[1].c_str());
			}
			*/
			else if (infofields[0] == "SVTYPE")
			{
				interval.svtype = infofields[1];
			}
		}
		
	} // if chr >= 1 && chr <= 22

//   cerr << "SV parsed" << interval.chr << ":" << interval.pos << "-" << interval.end << " depth " << tokens[9] << endl;

	X[0] = atof(tokens[9].c_str());
	
   // cerr << "First sample depth read" <<endl;

	for(int i=1;i<(int)vfs.size();++i)
	{
		vector<string> tks;
		split(lns[i].c_str(), " \t\n", tks);

		X[i] = atof(tks[9].c_str());

    //	cerr << i << "-th sample depth read" <<endl;
	}
	return 0;
} //invcfs::read_vcf

void read_intervals_from_vcf(vector<string> &sample_ids, vector<string> &vcf_files, vector<sv> &candidates)
{
//	for(int i=0;i<(int)sample_ids.size();++i)
int i=0; // TEMPORARY, READ SINGLE VCF FILE
	{
	//	cerr << "sample ID " << sample_ids[i] << endl;
//		cerr << "opening " << vcf_files[i] << endl;
		ifstream vfile(vcf_files[i].c_str(), ios::in);
		while(vfile.good())
		{
			string ln;
			getline(vfile, ln);


			if (!ln.empty())
			{
				if (ln[0] != '#')
				{

				//	cerr << ln << endl;

					int chr;
					vector<string> tokens;
					split(ln.c_str(), " \t\n", tokens);
					// Let's add error handling later
					if (tokens[0].substr(0,3) == "chr" || tokens[0].substr(0,3) == "Chr" )
					{
						tokens[0] = tokens[0].substr(3,2);
					}
					if (tokens[0] == "X")
					{
						chr = 23;
					}
					else if (tokens[0] == "Y")
					{
						chr = 24;
					}
					else if (tokens[0] == "M" || tokens[0] == "MT")
					{
						chr = 25;
					}
					else
					{
						try
						{
							chr = atoi(tokens[0].c_str());
						}
						catch(int e)
						{
							chr = 0;
					
						}
					}
//					cerr << "chr : " << chr << endl;
					if (chr >=1 && chr <=24)  // include chrs 1-22, X, Y
					{
						sv new_interval;
						new_interval.chr = tokens[0]; // Dec 1, 2017
						new_interval.chrnum = chr;
						new_interval.pos = atoi(tokens[1].c_str());
//						new_interval.ci_pos.first = -1;
//						new_interval.ci_pos.second = -1;
//						new_interval.ci_end.first = -1;
//						new_interval.ci_end.second = -1;

//						cerr << "pos : " << new_interval.pos << endl;

						string info = tokens[7];
						string chr2 = new_interval.chr;
						
						//cerr << "info : " << info << endl;

						vector<string> infotokens;
						split(info.c_str(), ";", infotokens);
						for(int j=0;j<(int)infotokens.size();++j)
						{
							vector<string> infofields;
							//cerr << infotokens[j] << endl;
							split(infotokens[j].c_str(), "=", infofields);
							if (infofields.size()>1)
							{
								if (infofields[0] == "END")
								{
									new_interval.end = atoi(infofields[1].c_str());
									//cerr << "\tEND: " << new_interval.end << endl;
								}
								/*
								else if (infofields[0] == "CIPOS")
								{
									vector<string> ci;
									split(infofields[1].c_str(), ",", ci);
									new_interval.ci_pos.first = atoi(ci[0].c_str());
									new_interval.ci_pos.second = atoi(ci[1].c_str());
									//cerr << "\tCIPOS: " << new_interval.ci_pos.first << " , " << new_interval.ci_pos.second << endl;

								}
								else if (infofields[0] == "CIEND")
								{
									vector<string> ci;
									split(infofields[1].c_str(), ",", ci);
									new_interval.ci_end.first = atoi(ci[0].c_str());
									new_interval.ci_end.second = atoi(ci[1].c_str());
									//cerr << "\tCIEND: " << new_interval.ci_end.first << " , " << new_interval.ci_end.second << endl;
								}
								*/
								else if (infofields[0] == "SVTYPE")
								{
									new_interval.svtype = infofields[1];

									//cerr << "\tSVTYPE: " << new_interval.svtype << endl;
								}
								else if (infofields[0] == "CHR2")
								{
									chr2 = infofields[1];
								}
							}
								
						}
						if (chr2 == new_interval.chr && new_interval.pos > 0 && new_interval.end > new_interval.pos && (new_interval.end - new_interval.pos)<=10000000 )  // TEMPORARY!! 10Mb Max!
						{
							candidates.push_back(new_interval);
						}
					} // if chr >= 1 && chr <= 22
				}
			}
		}
		vfile.close();
		
	}
} //read_intervals_from_vcf


void readFam(string sFamFile, map<string, unsigned> &hIdSex)
{
	ifstream inFile(sFamFile.c_str(), ios::in);

	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		if (!ln.empty())
		{
			vector<string> tokens;
			split(ln.c_str(), " \t\n", tokens);

			if (tokens[1].empty())
			{
				cerr << "Error: empty sample ID"<< endl;
				abort();
			}
			else if (tokens[4].empty())
			{
				cerr << "Error: empty sample SEX" << endl;
			}

			if (tokens[4] == "1" || tokens[4] == "2")
			{
				hIdSex[tokens[1]] = strtoul(tokens[4].c_str(), NULL, 10);
			}
		}
	}
	inFile.close();


}



