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
extern uint32_t CHR;

void readIndex(string sInFile, vector<string> &sampleIDs, vector<string> &sampleDirs, vector<string> &eventFiles, vector<string> &depthFiles)
{
	ifstream inFile(sInFile.c_str(), ios::in);

	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		if (!ln.empty())
		{
			vector<string> tokens;
			pFile::tokenizeLine(ln.c_str(), " \t\n", tokens);

			if (tokens[0].empty())
			{
				cerr << "Error: empty sample ID"<< endl;
				abort();
			}
			else if (tokens[1].empty())
			{
				cerr << "Error: empty sample DIR" << endl;
			}
			else if (tokens[2].empty())
			{
				cerr << "Error: cannot find event files" << endl;
			}
			else if (tokens[3].empty())
			{
				cerr << "Error: cannot find event files" << endl;
			}
			sampleIDs.push_back(tokens[0]);
			sampleDirs.push_back(tokens[1]);
			eventFiles.push_back(tokens[2]);
			depthFiles.push_back(tokens[3]);
		}
	}
	inFile.close();
}

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
			pFile::tokenizeLine(ln.c_str(), " \t\n", tokens);

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
	
	/*
void readDepth(vector<string> &depthFiles, vector<interval_t> &intervals, vector< vector<double> > &X, vector<double> &AvgDepth)
{
	for(unsigned i=0; i<depthFiles.size(); ++i)
	{
		pFile dFile;
		dFile.load(depthFiles[i].c_str());

//		cerr << "\rReading depth ...  " << i+1 << "/" << depthFiles.size() << "...";

		for(unsigned j=0; j<intervals.size(); ++j)
		{
			char buf[100];
			const char *line;
			uint64_t chr = intervals[j].first/(uint64_t)1e10;
			uint64_t startpos = (intervals[j].first)%(uint64_t)1e10;
			uint64_t endpos = (intervals[j].second)%(uint64_t)1e10;
			if (j%100 == 0)
			{
				cerr << "\rReading depth ...  " << i+1 << "/" << depthFiles.size() << "... interval" << j+1 << "/" << intervals.size();
			}
			sprintf(buf, "%lu:%lu-%lu", chr, startpos, endpos);
			dFile.updateRegion(buf);
			double sum = 0;
			double N = 0;
			while( (line = (char*)dFile.getLine()) != NULL)
			{
				vector<string> tokens;
				pFile::tokenizeLine(line," \t\n",tokens);
				if (tokens.size() != 3)
				{
					cerr << "Error reading depth file " << depthFiles[i] << endl;
					abort();
				}
				sum+=(double)atof(tokens[2].c_str());
				N++;
			}
			X[j][i] = sum/(N*AvgDepth[i]);
		}
	}
	cerr << endl;
}
*/

void readDepth(vector<string> &depthFiles, Interval &interval, vector< vector<double> > &D, vector<double> &AvgD, vector<double> &AvgDepth)
{
	for(unsigned i=0; i<depthFiles.size(); ++i)
	{
		// To Do: do this earlier and get vector of *pFile instead of file names
		pFile dFile;
		dFile.load(depthFiles[i].c_str());

//		cerr << "\rReading depth ...  " << i+1 << "/" << depthFiles.size() << "...";

		char buf[100];
		unsigned offset = 1000;
		const char *line;
		uint64_t chr = interval.T.first/(uint64_t)1e10;
		uint64_t startpos = (interval.T.first)%(uint64_t)1e10;
		uint64_t endpos = (interval.T.second)%(uint64_t)1e10;

		sprintf(buf, "%llu:%llu-%llu", chr, startpos-offset, endpos+offset);
		dFile.updateRegion(buf);
		double sum = 0;
		double N = 0;
		uint64_t j=0;

		while( (line = (char*)dFile.getLine()) != NULL)
		{
			vector<string> tokens;
			pFile::tokenizeLine(line," \t\n",tokens);
			if (tokens.size() != 3)
			{
				cerr << "Error reading depth file " << depthFiles[i] << endl;
				abort();
			}
			uint64_t pos = atoi(tokens[1].c_str());
			double val = (double)atof(tokens[2].c_str());

			if (pos >= startpos && pos <= endpos)
			{
				sum+=val/AvgDepth[i];
				N++;
			}

			D[i][j] = val;
			++j;
		}
		AvgD[i] = sum/N;
	}
	cerr << endl;
}

void readDepth(vector<string> &depthFiles, vector<Interval> &intervals, vector< vector<double> > &X, vector<double> &AvgDepth)
{
	for(unsigned i=0; i<depthFiles.size(); ++i)
	{
		pFile dFile;
		dFile.load(depthFiles[i].c_str());

//		cerr << "\rReading depth ...  " << i+1 << "/" << depthFiles.size() << "...";

		for(unsigned j=0; j<intervals.size(); ++j)
		{
			char buf[100];
			const char *line;
			uint64_t chr = intervals[j].T.first/(uint64_t)1e10;
			uint64_t startpos = (intervals[j].T.first)%(uint64_t)1e10;
			uint64_t endpos = (intervals[j].T.second)%(uint64_t)1e10;
			if (j%100 == 0)
			{
				cerr << "\rReading depth ...  " << i+1 << "/" << depthFiles.size() << "... interval" << j+1 << "/" << intervals.size();
			}
			sprintf(buf, "%llu:%llu-%llu", chr, startpos, endpos);
			dFile.updateRegion(buf);
			double sum = 0;
			double N = 0;
			while( (line = (char*)dFile.getLine()) != NULL)
			{
				vector<string> tokens;
				pFile::tokenizeLine(line," \t\n",tokens);
				if (tokens.size() != 3)
				{
					cerr << "Error reading depth file " << depthFiles[i] << endl;
					abort();
				}
				sum+=(double)atof(tokens[2].c_str());
				N++;
			}
			X[j][i] = sum/(N*AvgDepth[i]);
		}
	}
	cerr << endl;
}

void readDepthOrig(vector<string> &smIDs, vector<string> &smDirs, vector<interval_t> &intervals, vector< vector<double> > &X, vector<double> &AvgDepth)
{
	char buf[256];
	char cmd[512];

	for(unsigned i=0; i<smIDs.size(); ++i)
	{
		uint64_t chr = 1;
		sprintf(cmd, "/bin/bzcat %s/REF/coverageRefScore-chr%d-%s.tsv.bz2",smDirs[i].c_str(), (int)chr, smIDs[i].c_str());
		FILE *fp = popen(cmd, "r");

		cerr << "\rReading depth information for each interval in sample " << i << "/" << smIDs.size() ;
		for(unsigned j=0; j<intervals.size(); ++j)
		{
			uint64_t c = intervals[j].first/(uint64_t)1e10;
			uint64_t pos_start = intervals[j].first%(uint64_t)1e10;
			uint64_t pos_end = intervals[j].second%(uint64_t)1e10;
			
			if (c > chr)
			{
				pclose(fp);

				sprintf(cmd, "/bin/bzcat %s/REF/coverageRefScore-chr%d-%s.tsv.bz2",smDirs[i].c_str(), (int)c, smIDs[i].c_str());
				fp = popen(cmd, "r");
			}

			uint64_t pos = 0;
			double sum = 0;
			int N=0;
			while(pos<pos_end  && !feof(fp))
			{
				if (fgets(buf, 256, fp) != NULL)
				{
					if (strlen(buf)>1 && buf[0] != '#' && buf[0] != '>')
					{
						vector<string> tokens;
						pFile::tokenizeLine(buf, " \t\n", tokens);
						pos  = strtoul(tokens[0].c_str(), NULL, 10);
						if (pos>pos_start)
						{
							pos = atof(tokens[4].c_str());
							N++;
						}
					}
				}
			}
			if (N>0)
			{
				X[j][i] = (sum/(double)N) / AvgDepth[i];
			}
			else
			{
				X[j][i] = 0;
			}
		}
		fclose(fp);
	}
}


double getAvgDepth(string smID, string asmDir)
{
	// To do: check this file naming convention
	string sDepthFile = asmDir + "/CNV/depthOfCoverage_100000-" + smID + ".tsv";

	ifstream inFile(sDepthFile.c_str(), ios::in);
	int gcIdx = 4;
	double sum = 0;
	double N = 0;

	if (!inFile.good())
	{
		cerr << "Cannot open  " << sDepthFile<< endl;
		abort();
	}
	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		if (!ln.empty())
		{
			vector<string> tokens;
			pFile::tokenizeLine(ln.c_str(), " \t\n", tokens);
			if (tokens[0].at(0) == '>')
			{
				for(unsigned j=0; j<tokens.size() ;++j)
				{
					if (tokens[j].compare("gcCorrectedCvg") == 0)
					{
						gcIdx = j;
					}
				}
			}
			else if (tokens[0].substr(0,3).compare("chr") == 0)
			{
				sum += atof(tokens[gcIdx].c_str());
				N++;
			}
		}
	}
	inFile.close();

	return(sum/(double)N);
}

string getCNVsegmentFileName(string sampleID, string sampleDir)
{
	string sSegmentFile = sampleDir + "/CNV/cnvSegmentsBeta-" + sampleID + ".tsv";
	ifstream inFile(sSegmentFile.c_str(), ios::in);

	if (!inFile.good())
	{
		inFile.close();
		sSegmentFile = sampleDir + "/CNV/cnvSegmentsDiploidBeta-" + sampleID+ ".tsv";
		inFile.open(sSegmentFile.c_str(), ios::in);
		if (!inFile.good())
		{
			cerr << "Error: Cannot open neither " << sampleDir << "/CNV/cnvSegmentsBeta-" << sampleID << ".tsv nor " << sampleDir << "/CNV/cnvSegmentsDiploidBeta-" << sampleID <<".tsv" << endl;
			abort();
		}
	}
	return sSegmentFile;

}

void readInterval(string fName, int fType, vector<Interval> &del_events, vector<Interval> &dup_events)
{
	unsigned eventIdx = 0;
	unsigned chrIdx = 0;
	unsigned beginIdx = 0;
	unsigned typeIdx = 0;
	unsigned sourceIdx = 0;
	unsigned cipos1_idx = 0;
	unsigned cipos2_idx = 0;
	unsigned ciend1_idx = 0;
	unsigned ciend2_idx = 0;

	unsigned endIdx = 0; // column no. where deletion event is marked
	string delID = ""; // string that identifies deletion event in the eventIdx-th column
	string dupID = ""; // string that identifies deletion event in the eventIdx-th column
	string eventName = ""; // string that identifies deletion column in the header - for sanity check

	switch(fType)
	{
		case 1: // CNV segment file
			chrIdx = 0;
			beginIdx = 1;
			endIdx = 2;
			eventIdx = 6;
			delID = "-";
			dupID = "+";
			eventName = "calledCNVType";
			break;

		case 2: // event file from junctions2event
			chrIdx = 5;
			beginIdx = 6;
			endIdx = 7;
			eventIdx = 1;
			delID = "deletion";
			dupID = "tandem-duplication";
			eventName = "Type";
			break;

		case 3: // generic interval file in "Chr\tBegin\tEnd" format
			chrIdx = 0;
			beginIdx = 1;
			endIdx = 2;
			typeIdx = 3;
			sourceIdx = 4;
			cipos1_idx = 5;
			cipos2_idx = 6;
			ciend1_idx = 7;
			ciend2_idx = 8;

			// this type of file does not have event type
			break;

		default:
			// This should not happen
			cerr << "Cannot understand " << fType << " in readInterval() " << endl;
			abort();

	}

	ifstream inFile(fName.c_str(), ios::in);

	if (!inFile.good())
	{
		cerr << "Error: cannot open " << fName << endl;
	}

	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		uint64_t c = 0;

		if (!ln.empty() && ln.at(0) != '#')
		{
			vector<string> tokens;
			bool bDel=true;
			pFile::tokenizeLine(ln.c_str(), " \t\n", tokens);

			if (fType!=3 && tokens[0].at(0) == '>')
			{
				for(unsigned j=0; j<tokens.size() ;++j)
				{
					if (tokens[j].compare(eventName.c_str()) == 0)
					{
						eventIdx = j;
					}
				}
			}
			else if (fType <3)
			{
				if (tokens.size() > chrIdx && tokens.size() > eventIdx && tokens[chrIdx].length()>3 && tokens[chrIdx].substr(0,3).compare("chr") == 0 && tokens[eventIdx].compare(delID.c_str()) == 0)
				{
					c = strtoul(tokens[chrIdx].substr(3).c_str(), NULL, 10);
					bDel = true;
				}
				else if (tokens.size() > chrIdx && tokens.size() > eventIdx && tokens[chrIdx].length()>3 && tokens[chrIdx].substr(0,3).compare("chr") == 0 && tokens[eventIdx].compare(dupID.c_str()) == 0)
				{
					c = strtoul(tokens[chrIdx].substr(3).c_str(), NULL, 10);
					bDel = false;
				}
			}
			else if (tokens[0].at(0) != '#')
			{
				c = strtoul(tokens[chrIdx].c_str(), NULL, 10);
				bDel =  (tokens[typeIdx] == "DEL");
			}

			if ( (CHR==0 && c>0) || (CHR>0 && c==CHR))
			{
				Interval interval;
				interval.T.first = strtoul(tokens[beginIdx].c_str(), NULL, 10) + c*1e10;
				interval.T.second = strtoul(tokens[endIdx].c_str(), NULL, 10) + c*1e10;

				if (fType < 3)
				{
					interval.ci_pos.first = 0;
					interval.ci_pos.second = 0;
					interval.ci_end.first = 0;
					interval.ci_end.second = 0;
				}
				else
				{
					interval.ci_pos.first = strtoul(tokens[cipos1_idx].c_str(), NULL, 10);
					interval.ci_pos.second = strtoul(tokens[cipos2_idx].c_str(), NULL, 10);
					interval.ci_end.first = strtoul(tokens[ciend1_idx].c_str(), NULL, 10);
					interval.ci_end.second = strtoul(tokens[ciend2_idx].c_str(), NULL, 10);
				}

				switch(fType)
				{
					case 1:
						interval.source = "CG_CNV";
						break;
					case 2:
						interval.source = "CG_EVENT";
						break;
					case 3:
						interval.source = tokens[sourceIdx];
						break;
				}

				if (bDel)
				{
					interval.sv_type = "DEL";
					del_events.push_back(interval);
				}
				else
				{
					interval.sv_type = "DUP";
					dup_events.push_back(interval);
				}
			}
		}
	}
	inFile.close();
}

/*
void readInterval(string fName, int fType, vector<interval_t> &del_events, vector<interval_t> &dup_events)
{
	unsigned eventIdx = 0;
	unsigned chrIdx = 0;
	unsigned beginIdx = 0;
	unsigned endIdx = 0; // column no. where deletion event is marked
	string delID = ""; // string that identifies deletion event in the eventIdx-th column
	string dupID = ""; // string that identifies deletion event in the eventIdx-th column
	string eventName = ""; // string that identifies deletion column in the header - for sanity check

	switch(fType)
	{
		case 1: // CNV segment file
			chrIdx = 0;
			beginIdx = 1;
			endIdx = 2;
			eventIdx = 6;
			delID = "-";
			dupID = "+";
			eventName = "calledCNVType";
			break;

		case 2: // event file from junctions2event
			chrIdx = 5;
			beginIdx = 6;
			endIdx = 7;
			eventIdx = 1;
			delID = "deletion";
			dupID = "tandem-duplication";
			eventName = "Type";
			break;

		case 3: // generic interval file in "Chr\tBegin\tEnd" format
			chrIdx = 0;
			beginIdx = 1;
			endIdx = 2;
			// this type of file does not have event type
			break;

		default:
			// This should not happen
			cerr << "Cannot understand " << fType << " in readInterval() " << endl;
			abort();

	}

	ifstream inFile(fName.c_str(), ios::in);

	if (!inFile.good())
	{
		cerr << "Error: cannot open " << fName << endl;
	}

	while(inFile.good())
	{
		string ln;
		getline(inFile,ln);
		uint64_t c = 0;

		if (!ln.empty() && ln.at(0) != '#')
		{
			vector<string> tokens;
			bool bDel=true;
			pFile::tokenizeLine(ln.c_str(), " \t\n", tokens);

			if (fType!=3 && tokens[0].at(0) == '>')
			{
				for(unsigned j=0; j<tokens.size() ;++j)
				{
					if (tokens[j].compare(eventName.c_str()) == 0)
					{
						eventIdx = j;
					}
				}
			}
			else if (fType <3)
			{
				if (tokens.size() > chrIdx && tokens.size() > eventIdx && tokens[chrIdx].length()>3 && tokens[chrIdx].substr(0,3).compare("chr") == 0 && tokens[eventIdx].compare(delID.c_str()) == 0)
				{
					c = strtoul(tokens[chrIdx].substr(3).c_str(), NULL, 10);
					bDel = true;
				}
				else if (tokens.size() > chrIdx && tokens.size() > eventIdx && tokens[chrIdx].length()>3 && tokens[chrIdx].substr(0,3).compare("chr") == 0 && tokens[eventIdx].compare(dupID.c_str()) == 0)
				{
					c = strtoul(tokens[chrIdx].substr(3).c_str(), NULL, 10);
					bDel = false;
				}
			}
			else if (tokens[0].at(0) != '#')
			{
				c = strtoul(tokens[chrIdx].c_str(), NULL, 10);
				bDel = true;
			}

//			if (c>0 && (CHR>0 && c==CHR))
			if ( (CHR==0 && c>0) || (CHR>0 && c==CHR))
			{
				interval_t interval;
				interval.first = strtoul(tokens[beginIdx].c_str(), NULL, 10) + c*1e10;
				interval.second = strtoul(tokens[endIdx].c_str(), NULL, 10) + c*1e10;

				if (bDel)
				{
					del_events.push_back(interval);
				}
				else
				{
					dup_events.push_back(interval);
				}
			}
		}
	}
	inFile.close();
}
*/

void write_vcf_dup(FILE *fp, unsigned ID, vector<unsigned short>& gt, vector<unsigned>& GQ, Interval& interval, unsigned ac, unsigned ns, vector<double>& X, vector<double>& AvgDepth, vector<Gaussian>& C, bool bFilter)
{
	unsigned n_comp=C.size();
	int chr = (int)(interval.T.first/1e10);
	uint64_t pos = interval.T.first%(uint64_t)1e10;
	uint64_t svend = interval.T.second%(uint64_t)1e10;

	fprintf(fp,"%d\t", chr);
	fprintf(fp,"%llu\t", pos);

	fprintf(fp,"%d:%d\t.\t<DUP>\t.\t", chr,ID);
	if (bFilter)
	{
		fprintf(fp, "PASS");
	}
	else
	{
		fprintf(fp, "FAIL");
	}
	fprintf(fp,"\tIMPRECISE;SRC=%s;CIPOS=%d,%d;CIEND=%d,%d;VT=SV;END=%llu;SVLEN=%d;AC=%d;AF=%1.4f;AN=%d;NS=%d;SVTYPE=DUP", interval.source.c_str(), interval.ci_pos.first, interval.ci_pos.second, interval.ci_end.first, interval.ci_end.second, svend, (int)((int64_t)svend-(int64_t)pos), ac, (0.5*ac/ns), ns*2, ns);

	fprintf(fp,";CLUS=%d", n_comp);

	fprintf(fp,";MEAN=%1.4f",C[0].Mean);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Mean);
	}

	fprintf(fp,";STDEV=%1.4f",C[0].Stdev);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Stdev);
	}

	fprintf(fp,";PR=%1.4f",C[0].Alpha);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Alpha);
	}

	fprintf(fp,"\tGT:CN:DP:GQ" );
	for(unsigned j=0; j<gt.size(); ++j)
	{
		switch(gt[j])
		{
			case 0:
				fprintf(fp,"\t./.:");
				break;
			case 2:
				fprintf(fp,"\t0/0:");
				break;
			case 3:
				fprintf(fp,"\t0/1:");
				break;
			case 4:
				fprintf(fp,"\t1/1:");
				break;
			default:
				fprintf(fp,"\t./.:");
				break;
		}
		if (gt[j] == 0)
		{
			fprintf(fp, ".:");
		}
		else
		{
			if (gt[j]<=4)
				fprintf(fp, "%d:",gt[j]);
			else
				fprintf(fp, ".:");
		}

		fprintf(fp, "%u:", (unsigned)floor(X[j]*AvgDepth[j]));
		fprintf(fp, "%u", GQ[j]);
	}
	fprintf(fp, "\n");
}

/*
void write_vcf_dup(FILE *fp, unsigned ID, vector<unsigned short>& gt, vector<unsigned>& GQ, pair<uint64_t, uint64_t>& interval, unsigned ac, unsigned ns, vector<double>& X, vector<double>& AvgDepth, vector<Gaussian>& C)
{
	unsigned n_comp=C.size();
	int chr = (int)(interval.first/1e10);
	uint64_t pos = interval.first%(uint64_t)1e10;
	uint64_t svend = interval.second%(uint64_t)1e10;

	fprintf(fp,"%d\t", chr);
	fprintf(fp,"%ld\t", pos);
	fprintf(fp,"%d:%d\t.\t<CNV>\t.\tPASS\tIMPRECISE;VT=SV;END=%ld;AN=%d;NS=%d;SVTYPE=CNV", chr, ID, svend, ns*2, ns);
	fprintf(fp,";CLUS=%d", n_comp);

	fprintf(fp,";MEAN=%1.4f",C[0].Mean);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Mean);
	}

	fprintf(fp,";STDEV=%1.4f",C[0].Stdev);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Stdev);
	}

	fprintf(fp,";PR=%1.4f",C[0].Alpha);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Alpha);
	}

	fprintf(fp,"\tGT:CN:DP:CNQ" );
	for(unsigned j=0; j<gt.size(); ++j)
	{
		fprintf(fp, "\t./.:");
		if (gt[j] == 0)
		{
			fprintf(fp, ".:");
		}
		else
		{
			fprintf(fp, "%d:",gt[j]-1);
		}

		fprintf(fp, "%u:", (unsigned)floor(X[j]*AvgDepth[j]));
		fprintf(fp, "%u", GQ[j]);
	}
	fprintf(fp, "\n");
}
*/

void write_vcf(FILE *fp, unsigned ID, vector<unsigned short>& gt, vector< vector<unsigned> >& GL, vector<unsigned>& GQ, Interval &interval, unsigned ac, unsigned ns, vector<double>& X, vector<double>& AvgDepth, vector<Gaussian>& C, double BE, bool bFilter)
{
	// For Deletions
	unsigned n_comp=C.size();
	int chr = (int)(interval.T.first/1e10);
	uint64_t pos = interval.T.first%(uint64_t)1e10;
	uint64_t svend = interval.T.second%(uint64_t)1e10;

	fprintf(fp,"%d\t", chr);
	fprintf(fp,"%llu\t", pos);
	fprintf(fp,"%d:%d\t.\t<DEL>\t.\t", chr,ID);
	if (bFilter)
	{
		fprintf(fp, "PASS");
	}
	else
	{
		fprintf(fp, "FAIL");
	}

	fprintf(fp,"\tIMPRECISE;SRC=%s;CIPOS=%d,%d;CIEND=%d,%d;VT=SV;END=%llu;SVLEN=-%d;AC=%d;AF=%1.4f;AN=%d;NS=%d;P_OVERLAP=%1.8f;SVTYPE=DEL", interval.source.c_str(), interval.ci_pos.first, interval.ci_pos.second, interval.ci_end.first, interval.ci_end.second, svend, (int)((int64_t)svend-(int64_t)pos), ac, (0.5*ac/ns), ns*2, ns, BE);
	fprintf(fp,";CLUS=%d", n_comp);

	fprintf(fp,";MEAN=%1.4f",C[0].Mean);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Mean);
	}

	fprintf(fp,";STDEV=%1.4f",C[0].Stdev);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Stdev);
	}

	fprintf(fp,";PR=%1.4f",C[0].Alpha);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Alpha);
	}

	fprintf(fp,"\tGT:DP:GQ:PL" );
	for(unsigned j=0; j<gt.size(); ++j)
	{
		switch(gt[j])
		{
			case 0:
				fprintf(fp, "\t./.:");
				break;
			case 1:
				fprintf(fp, "\t0/0:");
				break;
			case 2:
				fprintf(fp, "\t0/1:");
				break;
			case 3:
				fprintf(fp, "\t1/1:");
				break;
		}
		fprintf(fp, "%u:", (unsigned)floor(X[j]*AvgDepth[j]));
		fprintf(fp, "%u:", GQ[j]);
		fprintf(fp, "%u,%u,%u", GL[j][0], GL[j][1], GL[j][2]);
	}
	fprintf(fp, "\n");
}

/*
void write_vcf(FILE *fp, unsigned ID, vector<unsigned short>& gt, vector< vector<unsigned> >& GL, vector<unsigned>& GQ, pair<uint64_t, uint64_t>& interval, unsigned ac, unsigned ns, vector<double>& X, vector<double>& AvgDepth, vector<Gaussian>& C, double BE)
{
	// For Deletions
	unsigned n_comp=C.size();
	int chr = (int)(interval.first/1e10);
	uint64_t pos = interval.first%(uint64_t)1e10;
	uint64_t svend = interval.second%(uint64_t)1e10;

	fprintf(fp,"%d\t", chr);
	fprintf(fp,"%ld\t", pos);
	fprintf(fp,"%d:%d\t.\t<DEL>\t.\tPASS\tIMPRECISE;VT=SV;END=%ld;AC=%d;AF=%1.4f;AN=%d;NS=%d;P_OVERLAP=%1.8f;SVTYPE=DEL", chr, ID, svend, ac, (0.5*ac/ns), ns*2, ns, BE);
	fprintf(fp,";CLUS=%d", n_comp);

	fprintf(fp,";MEAN=%1.4f",C[0].Mean);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Mean);
	}

	fprintf(fp,";STDEV=%1.4f",C[0].Stdev);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Stdev);
	}

	fprintf(fp,";PR=%1.4f",C[0].Alpha);
	for(unsigned i=1;i<n_comp;++i)
	{
		fprintf(fp,",%1.4f",C[i].Alpha);
	}

	fprintf(fp,"\tGT:DP:GQ:PL" );
	for(unsigned j=0; j<gt.size(); ++j)
	{
		switch(gt[j])
		{
			case 0:
				fprintf(fp, "\t./.:");
				break;
			case 1:
				fprintf(fp, "\t0/0:");
				break;
			case 2:
				fprintf(fp, "\t0/1:");
				break;
			case 3:
				fprintf(fp, "\t1/1:");
				break;
		}
		fprintf(fp, "%u:", (unsigned)floor(X[j]*AvgDepth[j]));
		fprintf(fp, "%u:", GQ[j]);
		fprintf(fp, "%u,%u,%u", GL[j][0], GL[j][1], GL[j][2]);
	}
	fprintf(fp, "\n");
}
*/


void Data::open(string sDataFile, unsigned &n_del, unsigned &n_dup, unsigned &n_sample, vector<string> &sampleIDs, vector<double> &AvgDepth)
{
	dFile.open(sDataFile.c_str(), ios::in);
	if (!dFile.good())
	{
		cerr << "Cannot open data file " << sDataFile << endl;
		abort();
	}
	dFile >> n_del;
	dFile >> n_dup;
//n_dup = 0;
	dFile >> n_sample;
	sampleIDs.resize(n_sample, "");
	for(unsigned i=0;i<n_sample;++i)
	{
		dFile >> sampleIDs[i];
	}

	AvgDepth.resize(n_sample,0);
	for(unsigned j=0; j<n_sample; ++j)
	{
		dFile >> AvgDepth[j]; 
	}
}

/*
void Data::read(unsigned n_interval, unsigned n_sample, vector<interval_t> &intervals,  vector< vector<double> > &X)
{
	intervals.resize(n_interval);

	if (!dFile.good())
	{
		cerr << "Cannot read datafile " <<  endl;
		abort();
	}
	for(unsigned i=0; i<n_interval; ++i)
	{
		dFile >> intervals[i].first;
		dFile >> intervals[i].second;

		for(unsigned j=0; j<n_sample; ++j)
		{
			dFile >> X[i][j];
		}
	}
//	dFile.close();
}
*/

void Data::read(unsigned n_interval, unsigned n_sample, vector<Interval> &intervals,  vector< vector<double> > &X)
{
	intervals.resize(n_interval);

	if (!dFile.good())
	{
		cerr << "Cannot read datafile " <<  endl;
		abort();
	}
	for(unsigned i=0; i<n_interval; ++i)
	{
		dFile >> intervals[i].T.first;
		dFile >> intervals[i].T.second;
		dFile >> intervals[i].sv_type;
		dFile >> intervals[i].source;
		dFile >> intervals[i].ci_pos.first;
		dFile >> intervals[i].ci_pos.second;
		dFile >> intervals[i].ci_end.first;
		dFile >> intervals[i].ci_end.second;

		for(unsigned j=0; j<n_sample; ++j)
		{
			dFile >> X[i][j];
		}
	}
}


/*
void Data::write(string sDataFile, vector<string> &sampleIDs, vector<interval_t> &del_intervals, vector<interval_t> &dup_intervals,  vector< vector<double> > &X, vector< vector<double> > &Y, vector<double> &AvgDepth)
{
	unsigned n_del = del_intervals.size();
	unsigned n_dup = dup_intervals.size();
	unsigned n_sample = AvgDepth.size();

	string fName = sDataFile + ".dump";

	ofstream dumpFile(fName.c_str(), ios::out);
	if (!dumpFile.good())
	{
		cerr << "Cannot open " << sDataFile << " to write. " << endl;
		abort();
	}
	dumpFile << n_del << "\t" << n_dup << "\t" << n_sample ;
	for(unsigned i=0;i<n_sample;++i)
	{
		dumpFile << "\t" << sampleIDs[i];
	}
	dumpFile << endl;

	dumpFile << AvgDepth[0];
	for(unsigned j=1; j<n_sample; ++j)
	{
		dumpFile << "\t" << AvgDepth[j]; 
	}
	dumpFile << endl;
	for(unsigned i=0; i<n_del; ++i)
	{
		dumpFile << del_intervals[i].first  << "\t" << del_intervals[i].second;

		for(unsigned j=0; j<n_sample; ++j)
		{
			dumpFile << "\t" << X[i][j];
		}
		dumpFile << endl;
	}
	for(unsigned i=0; i<n_dup; ++i)
	{
		dumpFile << dup_intervals[i].first  << "\t" << dup_intervals[i].second;

		for(unsigned j=0; j<n_sample; ++j)
		{
			dumpFile << "\t" << Y[i][j];
		}
		dumpFile << endl;
	}

	dumpFile.close();
}
*/

void Data::write(string sDataFile, vector<string> &sampleIDs, vector<Interval> &del_intervals, vector<Interval> &dup_intervals,  vector< vector<double> > &X, vector< vector<double> > &Y, vector<double> &AvgDepth)
{
	unsigned n_del = del_intervals.size();
	unsigned n_dup = dup_intervals.size();
	unsigned n_sample = AvgDepth.size();

	string fName = sDataFile + ".dump";

	ofstream dumpFile(fName.c_str(), ios::out);
	if (!dumpFile.good())
	{
		cerr << "Cannot open " << sDataFile << " to write. " << endl;
		abort();
	}
	dumpFile << n_del << "\t" << n_dup << "\t" << n_sample ;
	for(unsigned i=0;i<n_sample;++i)
	{
		dumpFile << "\t" << sampleIDs[i];
	}
	dumpFile << endl;

	dumpFile << AvgDepth[0];
	for(unsigned j=1; j<n_sample; ++j)
	{
		dumpFile << "\t" << AvgDepth[j]; 
	}
	dumpFile << endl;
	for(unsigned i=0; i<n_del; ++i)
	{
		dumpFile << del_intervals[i].T.first  << "\t" << del_intervals[i].T.second << "\t";
		dumpFile << del_intervals[i].sv_type << "\t" << del_intervals[i].source << "\t" ;
		dumpFile << del_intervals[i].ci_pos.first  << "\t" << del_intervals[i].ci_pos.second << "\t";
		dumpFile << del_intervals[i].ci_end.first  << "\t" << del_intervals[i].ci_end.second;

		for(unsigned j=0; j<n_sample; ++j)
		{
			dumpFile << "\t" << X[i][j];
		}
		dumpFile << endl;
	}
	for(unsigned i=0; i<n_dup; ++i)
	{
		dumpFile << dup_intervals[i].T.first  << "\t" << dup_intervals[i].T.second << "\t";
		dumpFile << dup_intervals[i].sv_type  << "\t" << dup_intervals[i].source << "\t";
		dumpFile << dup_intervals[i].ci_pos.first  << "\t" << dup_intervals[i].ci_pos.second << "\t";
		dumpFile << dup_intervals[i].ci_end.first  << "\t" << dup_intervals[i].ci_end.second ;

		for(unsigned j=0; j<n_sample; ++j)
		{
			dumpFile << "\t" << Y[i][j];
		}
		dumpFile << endl;
	}

	dumpFile.close();
}


void write_vcfheader(FILE *vcfFile, vector<string> &sampleIDs)
{
	fprintf(vcfFile,"##fileformat=VCFv4.1\n");
	fprintf(vcfFile,"##source=UM_CGI_CNV_pipeline_v0.1\n");
	fprintf(vcfFile,"##INFO=<ID=AC,Number=1,Type=Integer,Description=\"Number of alternative allele\">\n");
	fprintf(vcfFile,"##INFO=<ID=NS,Number=1,Type=Integer,Description=\"Number of Samples With Data\">\n");
	fprintf(vcfFile,"##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Total Depth\">\n");
	fprintf(vcfFile,"##INFO=<ID=AF,Number=A,Type=Float,Description=\"Allele Frequency\">\n");
	fprintf(vcfFile,"##INFO=<ID=IMPRECISE,Number=0,Type=Flag,Description=\"Imprecise structural variation\">\n");
	fprintf(vcfFile,"##INFO=<ID=END,Number=1,Type=Integer,Description=\"End position of the variant described in this record\">\n");
	fprintf(vcfFile,"##INFO=<ID=SVTYPE,Number=1,Type=String,Description=\"Type of structural variant\">\n");
//	fprintf(vcfFile,"##INFO=<ID=CLUS,Number=1,Type=IntegerDescription=\"Number of depth clusters\">\n");
//	fprintf(vcfFile,"##INFO=<ID=MEAN,Number=.,Type=,Description=\"Means of each depth cluster\">\n");
//	fprintf(vcfFile,"##INFO=<ID=STDEV,Number=.,Type=,Description=\"Std. Dev. of each depth cluster\">\n");
//	fprintf(vcfFile,"##INFO=<ID=PR,Number=.,Type=,Description=\"Probability of each depth cluster\">\n");
//	fprintf(vcfFile,"##ALT=<ID=CNV,Description=\"Copy number variable region\">\n");
	fprintf(vcfFile,"##ALT=<ID=DEL,Description=\"Deletion\">\n");
	fprintf(vcfFile,"##ALT=<ID=DUP,Description=\"Duplication\">\n");
	fprintf(vcfFile,"##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n");
	fprintf(vcfFile,"##FORMAT=<ID=CN,Number=1,Type=Integer,Description=\"Copy Number\">\n");
	fprintf(vcfFile,"##FORMAT=<ID=CNQ,Number=G,Type=Integer,Description=\"Copy Number Quality\">\n");
	fprintf(vcfFile,"##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">\n");
	fprintf(vcfFile,"##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n");
	fprintf(vcfFile,"##FORMAT=<ID=PL,Number=G,Type=Integer,Description=\"Genotype Likelihood\">\n");
	fprintf(vcfFile,"#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT");
	for(unsigned j=0; j<sampleIDs.size();++j)
	{
		fprintf(vcfFile,"\t%s",sampleIDs[j].c_str());
	}
	fprintf(vcfFile,"\n");

}
