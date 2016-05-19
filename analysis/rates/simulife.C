#include <TRandom.h>
#include <TMath.h>

const double ln2   = 0.69314718055994530942;

// year
double sec  = 1;
double min  = 60*sec;
double hour = 60*min;
double day  = 24*hour;
double year = 365*day;

// lifetime of the isotope in seconds
double thalf = 1925.20 * day;
// rate at t=0
double R0 = 30 / sec; // Hz
// timestep in seconds
double deltat = 900*sec;
// number of pseudo experiments
int n_pseudo = 100;
// measuring time
// // double tmax = 2.*year;

Double_t gaus(Double_t *v, Double_t *par)
{
    Double_t arg = 0;
    if (par[2] != 0) arg = (v[0] - par[1])/par[2];
    
    Double_t fitval = par[0]*TMath::Exp(-0.5*arg*arg);
    fitval += par[3] + par[4]*v[0] + par[5]*v[0]*v[0];
    
    return fitval;
}

Double_t fitf(Double_t *v, Double_t *par)
{
    Double_t arg = v[0];
    Double_t fitval = par[0]*TMath::Exp(-ln2*arg/(par[1]*day));
    
    return fitval;
}

double rate(double time_sec){
    double lambda = ln2 / thalf;
    return R0*exp(-lambda*time_sec);
}

void simulife(){
    char cmd[256];
    cout << "simulife:: lifetime measurement simulator" <<endl;
    
    TRandom *_r = new TRandom();
    
    const int np = 100;
    double tt[np];
    double ss[np];
    
    for (int ip = 0; ip<np; ip++){
        
        double tmax = (ip+1)*0.02*year;
        
        TProfile *_life = new TProfile("_life","_life",50,0.,tmax,0.,R0*1.5);
        
        TH1F *_thalf = new TH1F("thalf","thalf",50,thalf/day-400.,thalf/day+400.);
        TH1F *_delta = new TH1F("delta","delta",50,-400.,400.);
        
        cout << n_pseudo<<endl;
        for (int i=0; i<n_pseudo; i++){
            if(i%10==0) cout <<" pseudo experiment "<<i<< " tmax = "<<tmax<<endl;
            // simulate 1 data set
            double time = 0;
            while (time < tmax){
                double R     = rate(time);
                double N_th  = R*deltat;
                double N_exp = N_th + _r->Gaus(0,TMath::Sqrt(N_th));
                double R_exp = N_exp / deltat;
                
                _life->Fill(time,R_exp);
                time += deltat;
            }
            
            TF1 *func = new TF1("fit",fitf,0,tmax,2);
            func->SetParameters(R0,thalf/day);
            func->SetParLimits(0,R0-10,R0+10);
            func->SetParLimits(1,thalf/day-5*365,thalf/day+5*365);
            
            _life->Fit("fit","0Q");
            
            double thalf_exp = func->GetParameter(1);
            _delta->Fill((thalf_exp - thalf/day)/(thalf/day));
            _thalf->Fill(thalf_exp);
            _life->Reset();
        }
        
        int maxbin = _thalf->GetMaximumBin();
        double maxval = _thalf->GetBinContent(maxbin);
        TF1 *func1 = new TF1("gaus1",gaus,thalf/day-400,thalf/day+400,3);
        func1->SetParameters(maxval,thalf/day,_thalf->GetRMS());
        func1->SetParLimits(2,0,500);
        
        _thalf->Fit("gaus1");

        gStyle->SetOptFit(111);
        sprintf(cmd,"%d pseudo experiments, rate(t=0) = %3.1f Hz, t1/2 = %5.2f days, t_meas = % 5.2f yr",
                n_pseudo,R0,thalf/day,tmax/year);
        _thalf->SetTitle(cmd);
        _thalf->Draw();
        _thalf->GetXaxis()->SetTitle("Lifetime (days)");
        
        tt[ip] = tmax / year;
        ss[ip] = func1->GetParameter(2);
        
    }
    
    TGraph *g1 = new TGraph(np,tt,ss);
    g1->SetMarkerStyle(24);
    g1->Draw("APL");
    
    c1->Update();
    
}
