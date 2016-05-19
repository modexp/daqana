#include <string>
//
// calplot.C - Plot the calibration as a function of time. The energy of 
//             a signal of 0.2e-6Vs is calculated.
//
// Arguments:
//                cal_file    - directory with calibration root file(s)
//                plot-type   - what to plot - (1) "trend" (2) "chi2"
//                ich         - channel number
//
// A.P. Colijn

//float chi2_cut[8] = {1.8,2.5,9.5,2.5,7.5,2.5,3.5,5.5};

void calplot(string cal_file, string plot_type, int ich){
   char cmd[128],cut[128];
   //
   // make a chain with all the calibration files you want....
   //
   TChain *run = new TChain("cal");
   sprintf(cmd,"%s*.root",cal_file.c_str());
   cout <<"cmd >"<<cmd<<"<<"<<endl;
   run->Add(cmd);

   if        (plot_type == "chi2"){
      //
      // plot the chi2 of the calibration fit
      //
      run->SetMarkerStyle(24);
      run->SetMarkerColor(1);
      //sprintf(cmd,"chi2[%i]:cal_tmin",ich);
      //run->Draw(cmd,"","prof");
      sprintf(cmd,"chi2[%i]",ich);
      run->Draw(cmd,"","");
   } else if (plot_type == "trend"){
      //
      // plot the energy of a 0.2e-6Vs signal as a function of time
      //
      sprintf(cmd,"c0[%i]+c1[%i]*0.2e-6:cal_tmin",ich,ich);
//      sprintf(cut,"chi2[%i]>%f",ich,chi2_cut[ich]);
      cout<<"cmd = "<<cmd<<endl;
//      cout<<"cut = "<<cut<<endl;

      run->SetMarkerStyle(24);
      run->SetMarkerColor(1);
      run->Draw(cmd);

//      run->SetMarkerStyle(24);
//      run->SetMarkerColor(2);
//      run->Draw(cmd,cut,"psame");
   }
}
