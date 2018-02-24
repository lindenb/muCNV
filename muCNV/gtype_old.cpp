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

extern double BE_THRESHOLD;
extern double P_THRESHOLD;

bool ordered(vector<Gaussian> &C)
{
	for(unsigned i=0; i<C.size()-1;++i)
	{
		if (C[i].Mean<=C[i+1].Mean)
			return false;
	}
	return true;
}


void gtype::call_del(sv &s, vector<double> &X, vector<double> &Y, vector<int> &geno, outvcf& v, vector<double> &AvgDepth)
{
	int n = (int)X.size();
	
	vector< vector<int> > GL(n, vector<int>(3,255));
	vector<int> GQ(n, 0);
	vector<double> bic(3,0);
	
	vector<Gaussian> C1(1); // 1-component model
	vector<Gaussian> C2(2); // 2-component model
	vector<Gaussian> C3(3); // 3-component model
	
	// For each candidate region, run EM
	// Run EM with 2 and 3 components, compare likelihoods with the 1-gaussian model, apply BIC
	bool bFlip = false;
	
	// Single-component model
	C1[0].Mean = mean(X);
	C1[0].Stdev = stdev(X, C1[0].Mean);
	C1[0].Alpha = 1;
	
	bic[0] = BIC(X, C1);
	
	// Two-component model
	C2[0].Mean = 1;
	C2[1].Mean = 0.5;
	C2[0].Stdev = C2[1].Stdev = 0.1;
	C2[0].Alpha = C2[1].Alpha = 0.5;
	
	// Three-component model
	C3[0].Mean = 1;
	C3[1].Mean = 0.5;
	C3[2].Mean = 0;
	C3[0].Stdev = C3[1].Stdev = C3[2].Stdev = 0.1;
	C3[0].Alpha = C3[1].Alpha = C3[2].Alpha = 1.0/3.0;
	
	EM(X, C3, bFlip);
	bic[2] = BIC(X, C3);
	
	// check which one is major
	if (C3[2].Mean < C3[0].Mean && C3[2].Alpha > C3[0].Alpha)
	{
		
		bFlip = true;
		C2[0].Mean = 0.5;
		C2[1].Mean = 0;
		
		EM(X, C2, bFlip);
	}
	else
	{
		// Two-component EM and BIC
		EM(X, C2, bFlip);
	}
	bic[1] = BIC(X, C2);
	
	// Determine whether this is a polymorphic interval or not, make calls
	if (bic[0]>bic[1] && bic[2]>bic[1])
	{
		// 2 cluster
		double BE = BayesError(C2);
		min_bic = bic[1];
		p_overlap = BE;
		if (ordered(C2) && BE< BE_THRESHOLD)
		{
			int NS=0;
			int AC = classify_del(X, geno, GL, GQ, NS, C2, bFlip);
			
			//		cerr << "AC: " << AC << endl;
			//			if (AC>0)
			{
				v.write_del(s, geno, GQ, AC, NS, X, AvgDepth, C2, BE, true);
			}
		}
		else if (ordered(C2))
		{
			int NS=0;
			int AC = classify_del(X, geno, GL, GQ, NS, C2, bFlip);
			
			//		cerr << "AC: " << AC << endl;
			//			if (AC>0)
			{
				v.write_del(s, geno, GQ, AC, NS, X, AvgDepth, C2, BE, false);
			}
			
		}
		
		// Two-component is selected
		//	printCluster(C2);
	}
	else if (bic[2]<bic[0] && bic[2]<bic[1])
	{
		// 3 cluster
		double BE = BayesError(C3);
		min_bic = bic[2];
		p_overlap = BE;
		if (ordered(C3) && BE< BE_THRESHOLD)
		{
			int NS=0;
			int AC = classify_del(X, geno, GL, GQ, NS, C3, bFlip);
			
			//			if (AC>0)
			{
				v.write_del(s, geno, GQ, AC, NS, X, AvgDepth, C3, BE, true);
			}
		}
		else if (ordered(C3))
		{
			int NS=0;
			int AC = classify_del(X, geno, GL, GQ, NS, C3, bFlip);
			
			//			if (AC>0)
			{
				v.write_del(s, geno, GQ, AC, NS, X, AvgDepth, C3, BE, false);
			}
		}
		
		//	printCluster(C3);
	}
	else  // TODO: Write more about filtering reasons
	{
		if (bic[1] < bic[2])
		{
			double BE = BayesError(C2);
			
			int NS=0;
			int AC = classify_del(X, geno, GL, GQ, NS, C2, bFlip);
			
			v.write_del(s, geno, GQ, AC, NS, X, AvgDepth, C2, BE, false);
		}
		else
		{
			double BE = BayesError(C3);
			
			int NS=0;
			int AC = classify_del(X, geno, GL, GQ, NS, C3, bFlip);
			
			v.write_del(s, geno, GQ, AC, NS, X, AvgDepth, C3, BE, false);
			
		}
	}
}

void gtype::call_cnv(sv &s, vector<double> &X, vector<double> &Y, vector<int> &geno, outvcf& v, vector<double> &AvgDepth)
{
	size_t n = X.size();
	
	vector< vector<int> > GL(n, vector<int>(3,255));
	vector<int> GQ(n, 0);
	
	// For each candidate region, run EM
	// Run EM with 2 and 3 components, compare likelihoods with the 1-gaussian model, apply BIC
	vector<Gaussian> Comps(1);
	
	double min_BIC = DBL_MAX;
	vector<double> bic(10,0);
	
	double M = mean(X);
	double S = stdev(X, M);
	
	Comps[0].Mean = M;
	Comps[0].Stdev = S;
	Comps[0].Alpha = 1;
	
	min_BIC = bic[0] = BIC(X, Comps);
	//		cerr << "1 comp, bic " << min_BIC << endl;
	unsigned n_comp = 1;
	
	// Check up to 30 -- should be enough? -- let's go for 10 now
	// It seems like too many components make probs too small hence failing
	
	for(unsigned i=2; i<10; ++i)
	{
		Gaussian* pG = new Gaussian;
		Comps.push_back(*pG);
		
		for(unsigned j=0; j<i; ++j)
		{
			Comps[j].Mean = 1.0 + (double)j*0.5;
			Comps[j].Stdev = 0.1;
			Comps[j].Alpha = 1;
		}
		// Run EM with i components
		EM(X, Comps);
		
		bic[i-1] = BIC(X, Comps);
		
		if (bic[i-1]<min_BIC)
		{
			min_BIC = bic[i-1];
			n_comp = i;
		}
	}
	
	Comps.resize(n_comp);
	
	// Determine whether this is a polymorphic interval or not, make calls
	
	//		cerr << "n_comp: " << n_comp  << endl;
	
	for(unsigned j=0;j<n_comp;++j)
	{
		Comps[j].Mean = 1.0 + (double)j*0.5;
		Comps[j].Stdev = 0.1;
		Comps[j].Alpha = 1;
	}
	EM(X, Comps);
	double BE = BayesError(Comps);
	if (BE<BE_THRESHOLD)
	{
		int NS=0;
		int AC = classify_cnv(X, geno, GQ, NS, Comps);
		//		if (AC>0)
		{
			// ++write_cnt;
			v.write_cnv(s, geno, GQ, AC, NS, X, AvgDepth, Comps, BE,  true);
		}
	}
	else
	{
		int NS=0;
		int AC = classify_cnv(X, geno, GQ, NS, Comps);
		//		if (AC>0)
		{
			// ++write_cnt;
			v.write_cnv(s, geno, GQ, AC, NS, X, AvgDepth, Comps, BE, false);
		}
		
	}
	
}


int gtype::classify_del(vector<double>& x, vector<int>& GT, vector< vector<int> >& GL, vector<int>& GQ, int& ns, vector<Gaussian>& C, bool bFlip)
{
	unsigned n_sample = (unsigned) x.size();
	unsigned n_comp = (unsigned) C.size();
	unsigned ac = 0;
	ns = 0;
	
	for(unsigned j=0; j<n_sample; ++j)
	{
		double max_pos = -1.0 * DBL_MAX;
		double max_lk = -1.0 * DBL_MAX;
		double lk[n_comp];
		double pos[n_comp];
		double sum_pos = 0;
		double sum_lk = 0;
		unsigned short gt=0;
		for(unsigned m=0; m<n_comp; ++m)
		{
			//posterior
			lk[m]  = normpdf(x[j], C[m]);
			pos[m] = C[m].Alpha * lk[m];
			
			sum_pos += pos[m];
			sum_lk += lk[m];
			
			if (lk[m]>max_lk)
			{
				if (bUseGL) gt = m+1;
				max_lk = lk[m];
			}
			if (pos[m]>max_pos)
			{
				if (!bUseGL) gt = m+1;
				max_pos = pos[m];
			}
		}
		
		if (bFlip && n_comp==2)
		{
			++gt;
		}
		for(unsigned m=0; m<n_comp; ++m)
		{
			if (sum_lk>1e-100)
			{
				if (lk[m] < 8.4e-12)
				{
					GL[j][m] = 255;
				}
				else
				{
					GL[j][m] = -10.0*log10(lk[m]/sum_lk);
					if (GL[j][m]>255)
					{
						cerr << "error: lk is " << lk[m] << " and sum_lk is " << sum_lk << ", GL is " << GL[j][m] << endl;
					}
				}
			}
			else
			{
				GL[j][m] = 255;
			}
		}
		
		if (n_comp==2)
		{
			if (bFlip)
			{
				GL[j][0] = 255;
			}
			else
			{
				GL[j][2] = 255;
			}
		}
		
		double max_p = (bUseGL) ? max_lk : max_pos;
		double sum_p = (bUseGL) ? sum_lk : sum_pos;
		
		if (max_p / sum_p > P_THRESHOLD && sum_p > 1e-100)
		{
			GT[j] = gt;
			if (gt>0)
			{
				ac += (gt-1);
				++ns;
			}
			double p_err = (sum_p-max_p) / sum_p;
			
			if (p_err > 8.4e-12)
			{
				GQ[j] = (unsigned)floor(-10.0 * log10((sum_p - max_p) / (sum_p) ));
				
				if (GQ[j]>255)
				{
					cerr << "error: max_p is " << max_p << " and sum_p is " << sum_p << ", GQ is " << GQ[j] << endl;
				}
			}
			else
			{
				GQ[j] = 255;
			}
		}
		else
		{
			GT[j] = 0;
			
			if (sum_p>1e-100)
			{
				double p_err = (sum_p-max_p) / sum_p;
				
				if (p_err > 8.4e-12)
				{
					GQ[j] = (unsigned)floor(-10.0 * log10 ((sum_p - max_p) / (sum_p) ));
					
					if (GQ[j]>255)
					{
						cerr << "error: max_p is " << max_p << " and sum_p is " << sum_p << ", GQ is " << GQ[j] << endl;
					}
				}
				else
				{
					GQ[j] = 255;
				}
			}
			else
			{
				GQ[j] = 0;
			}
			
		}
	}
	return ac;
}


int gtype::classify_cnv(vector<double>& x, vector<int>& GT, vector<int>& GQ, int& ns, vector<Gaussian>& Comps)
{
	unsigned n_sample = (unsigned) x.size();
	unsigned n_comp = (unsigned) Comps.size();
	unsigned ac = 0;
	ns = 0;
	
	for(unsigned j=0; j<n_sample; ++j)
	{
		double max_pos = -1.0 * DBL_MAX;
		double max_lk = -1.0 * DBL_MAX;
		double lk[n_comp];
		double pos[n_comp];
		double sum_pos = 0;
		double sum_lk = 0;
		unsigned short gt=0;
		
		for(unsigned m=0; m<n_comp; ++m)
		{
			//posterior
			lk[m]  = normpdf(x[j], Comps[m] );
			pos[m] = lk[m]; //TEMPORARY, USE LIKELIHOOD
							//	pos[m] = Comps[m].Alpha * lk[m];
			
			sum_pos += pos[m];
			sum_lk += lk[m];
			
			if (lk[m]>max_lk)
			{
				max_lk = lk[m];
			}
			if (pos[m]>max_pos)
			{
				gt = round(Comps[m].Mean*2.0);
				max_pos = pos[m];
			}
		}
		
		if (gt<2 || gt>4)
		{
			// non-biallelic, let's handle this later
			return 0;
		}
		
		double max_p = max_pos;
		double sum_p = sum_pos;
		
		if (max_p / sum_p > P_THRESHOLD && sum_p > 1e-100)
		{
			
			//			cerr << "GOOD: max_p is " << max_p << " and sum_p is " << sum_p << ", P_THRESHOLD is " << P_THRESHOLD << " GT is " << gt << endl;
			GT[j] = gt;
			if (gt>2)
			{
				ac += (gt - 2);
				++ns;
			}
			double p_err = (sum_p-max_p) / sum_p;
			
			if (p_err > 8.4e-12)
			{
				GQ[j] = (unsigned)floor(-10.0 * log10((sum_p - max_p) / (sum_p) ));
				
				if (GQ[j]>255)
				{
					GQ[j] = 255;
					//cerr << "error: max_p is " << max_p << " and sum_p is " << sum_p << ", GQ is " << GQ[j] << endl;
				}
			}
			else
			{
				GQ[j] = 255;
			}
		}
		else
		{
			
			//			cerr << "BAD: max_p is " << max_p << " and sum_p is " << sum_p << ", P_THRESHOLD is " << P_THRESHOLD << " GT is " << gt << endl;
			GT[j] = 0;
			if (sum_p>1e-100)
			{
				double p_err = (sum_p-max_p) / sum_p;
				
				if (p_err > 8.4e-12)
				{
					GQ[j] = (unsigned)floor(-10.0 * log10 ((sum_p - max_p) / (sum_p) ));
					
					if (GQ[j]>255)
					{
						GQ[j] = 255;
						//		cerr << "error: max_p is " << max_p << " and sum_p is " << sum_p << ", GQ is " << GQ[j] << endl;
					}
				}
				else
				{
					GQ[j] = 255;
				}
			}
			else
			{
				GQ[j] = 0;
			}
		}
	}
	return ac;
}

void gtype::EM2(vector<double>& x, vector<double> &y, vector<Gaussian2>& C)
{
	
	// Let's not consider half-normal distribution -- for now
	
	unsigned n_sample = (unsigned) x.size();
	unsigned n_comp = (unsigned) C.size();
	
	unsigned n_iter = 30;
	// pseudo-counts
	unsigned p_count= 2;
	double p_val[n_comp][2];
	
	for(unsigned i=0; i<n_comp; ++i)
	{
		p_val[i][0] = C[i].Mean[0];
		p_val[i][1] = C[i].Mean[1];
	}
	
	// pseudo-means
	//	double p_val[3] = {1.0, 0.5, 0};
	
	for(unsigned i=0; i<n_iter; ++i)
	{
		vector<double> sum_x (n_comp,0);
		vector<double> sum_y (n_comp,0);
		
		vector<double> sum_e_xx (n_comp,0);
		vector<double> sum_e_xy (n_comp,0);
		vector<double> sum_e_yy (n_comp,0);
		
		vector<double> sum_pr (n_comp,0);
		
		// E step
		for(unsigned j=0; j<n_sample; ++j)
		{
			double sum_p = 0;
			vector<double> pr(n_comp, 0);
			for(unsigned m=0;m<n_comp;++m)
			{
				pr[m] = C[m].Alpha * C[m].pdf(x[j], y[j]);
				sum_p += pr[m];
			}
			
			// M step
			
			if (sum_p > 1e-100) // if the value is an outlier, exclude it from calculations
			{
				for(unsigned m=0;m<n_comp;++m)
				{
					pr[m] /= sum_p;
					sum_x[m] += pr[m] * x[j];
					sum_y[m] += pr[m] * y[j];
					
					double ex = x[j] - C[m].Mean[0];
					double ey = y[j] - C[m].Mean[1];
					
					sum_e_xx[m] += pr[m] * ex * ex;
					sum_e_xy[m] += pr[m] * ex * ey;
					sum_e_yy[m] += pr[m] * ey * ey;
					sum_pr[m] += pr[m];
				}
			}
		}
		
		// Add pseudo-count values
		/*
		 for(unsigned m=0; m<n_comp; ++m)
		 {
		 sum_x[m] += p_val[m][0] * p_count;
		 sum_y[m] += p_val[m][1] * p_count;
		 
		 sum_e_xx[m] += (p_val[m][0] - C[m].Mean[0])*(p_val[m][0] - C[m].Mean[0]) * p_count;
		 sum_e_xy[m] += (p_val[m][0] - C[m].Mean[0])*(p_val[m][1] - C[m].Mean[1]) * p_count;
		 sum_e_yy[m] += (p_val[m][1] - C[m].Mean[1])*(p_val[m][1] - C[m].Mean[1]) * p_count;
		 sum_pr[m] += p_count;
		 }
		 */
		for(unsigned m=0;m<n_comp;++m)
		{
			C[m].Mean[0] = sum_x[m]/sum_pr[m];
			C[m].Mean[1] = sum_y[m]/sum_pr[m];
			
			C[m].Cov[0] = sum_e_xx[m] / sum_pr[m];
			C[m].Cov[1] = C[m].Cov[2] = sum_e_xy[m] / sum_pr[m];
			C[m].Cov[3] = sum_e_yy[m] / sum_pr[m];
			C[m].Alpha = sum_pr[m] / (n_sample );
			
			//C[m].Alpha = sum_pr[m] / (n_sample + n_comp*p_count);
			C[m].update();
			
			
		}
	}
}


// EM for deletions
void gtype::EM(vector<double>& x, vector<Gaussian>& C, bool bFlip)
{
	unsigned n_sample = (unsigned) x.size();
	unsigned n_comp = (unsigned) C.size();
	
	unsigned n_iter = 30;
	// pseudo-counts
	unsigned p_count= 2;
	// pseudo-means
	double p_val[3] = {1.0, 0.5, 0};
	
	if (bFlip && n_comp==2)
	{
		// if fitting for (0, 1) copies, set pseudo-means accordingly
		p_val[0] = 0.5;
		p_val[1] = 0;
	}
	
	for(unsigned i=0; i<n_iter; ++i)
	{
		vector<double> sum (n_comp,0);
		vector<double> sum_pr (n_comp,0);
		vector<double> sum_err (n_comp,0);
		
		
		// E step
		for(unsigned j=0; j<n_sample; ++j)
		{
			double sum_p = 0;
			vector<double> pr(n_comp, 0);
			for(unsigned m=0;m<n_comp;++m)
			{
				pr[m] = C[m].Alpha * normpdf(x[j], C[m]);
				if (m==2)
				{
					// half-normal scaler
					pr[m] *= 2.0;
				}
				else if (bFlip && n_comp==2 &&  m==1)
				{
					pr[m] *=2.0;
				}
				sum_p += pr[m];
			}
			
			if (sum_p > 1e-100) // if the value is an outlier, exclude it from calculations
			{
				for(unsigned m=0;m<n_comp;++m)
				{
					pr[m] /= sum_p;
					sum[m] += pr[m] * x[j];
					sum_err[m] += pr[m] * (x[j] - C[m].Mean)*(x[j]-C[m].Mean);
					sum_pr[m] += pr[m];
				}
			}
		}
		
		// Add pseudo-count values
		for(unsigned m=0;m<n_comp; ++m)
		{
			
			
			sum[m] += p_val[m] * p_count;
			sum_err[m] += (p_val[m] - C[m].Mean)*(p_val[m] - C[m].Mean) * p_count;
			sum_pr[m] += p_count;
		}
		cerr << endl;
		
		// M step
		for(unsigned m=0;m<n_comp;++m)
		{
			C[m].Mean = sum[m]/sum_pr[m];
			C[m].Stdev = sqrt(sum_err[m] / (sum_pr[m] ));
			C[m].Alpha = sum_pr[m] / (n_sample + n_comp*p_count);
			if (m==2)
			{
				// half-normal with mean 0 and sigma
				C[m].Mean = 0;
			}
			else if (bFlip && n_comp==2 && m==1)
			{
				C[m].Mean = 0;
			}
			
		}
	}
}
