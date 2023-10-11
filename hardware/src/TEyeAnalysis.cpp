#include "TEyeAnalysis.h"
#include "TArrow.h"
#include "TCanvas.h"
#include "TEyeMeasurement.h"
#include "TFile.h"
#include "TH2.h"
#include "TLatex.h"
#include <fstream>
#include <iomanip>
#include <sstream>

TEyeAnalysis::TEyeAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                           std::vector<THic *> hics, std::mutex *aMutex, TEyeResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TEyeResult();
  FillVariableList();
}

void TEyeAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
}


void TEyeAnalysis::AnalyseHisto(TScanHisto *histo)
{
  std::cout << "in analyse histo, chipList.size = " << m_chipList.size() << std::endl;
  FILE *fp = fopen("EyeDiagram.dat", "w");

  // assume values from first chip for all
  TEyeResultHic *hicResult_0 = (TEyeResultHic *)FindHicResultForChip(m_chipList.at(0));
  Int_t driverStrength_0     = ((TEyeParameters *)hicResult_0->GetScanParameters())->driverStrength;
  Int_t preemphasis_0        = ((TEyeParameters *)hicResult_0->GetScanParameters())->preemphasis;

  const std::time_t t = time(nullptr);
  char              time_suffix[50];
  strftime(time_suffix, sizeof(time_suffix), "%y%m%d_%H%M%S", std::localtime(&t));

  std::string basename =
      TString::Format("eye_D%02i_P%02i_%s", driverStrength_0, preemphasis_0, time_suffix).Data();
  std::string filename_eye      = hicResult_0->GetOutputPath() + "/" + basename + ".pdf";
  std::string filename_eye_root = hicResult_0->GetOutputPath() + "/" + basename + ".root";

  TCanvas c;
  c.cd();
  c.Print((filename_eye + "[").c_str());

  TFile *rootfile_eye = TFile::Open(filename_eye_root.c_str(), "RECREATE");

  for (auto chip : m_chipList) {
    const double step_x = histo->GetStep(chip, 0);
    const double step_y = histo->GetStep(chip, 1);

    const double min_x = histo->GetMin(chip, 0);
    const double min_y = histo->GetMin(chip, 1);

    const int nbin_x = histo->GetNBin(chip, 0) - 1;
    const int nbin_y = histo->GetNBin(chip, 1) - 1;

    const int    nbin_x_half = nbin_x / 2;
    const int    nbin_y_half = nbin_y / 2;
    const int    xband       = 2;
    const int    yband       = 2;
    const double percent     = 0.98;

    TEyeResultHic *hicResult = (TEyeResultHic *)FindHicResultForChip(chip);
    Int_t driverStrength     = ((TEyeParameters *)hicResult->GetScanParameters())->driverStrength;
    Int_t preemphasis        = ((TEyeParameters *)hicResult->GetScanParameters())->preemphasis;

    const std::string hname =
        TString::Format("h_eye_%i_d%02i_p%02i", chip.chipId, driverStrength, preemphasis).Data();
    const std::string htitle =
        TString::Format("Eye Diagram chip %i (%s - D: %i, P: %i);%s;%s", chip.chipId,
                        hicResult->GetName().c_str(), driverStrength, preemphasis, "offset_{hor}",
                        "offset_{ver}")
            .Data();

    TH2F h_eye(hname.c_str(), htitle.c_str(), nbin_x, min_x - .5 * step_x,
               min_x - .5 * step_x + nbin_x * step_x, nbin_y, min_y - .5 * step_y,
               min_y - .5 * step_y + nbin_y * step_y);
    h_eye.SetDirectory(0);

    // fill histogram
    for (int xbin = 0; xbin < nbin_x; xbin++) {
      for (int ybin = 0; ybin < nbin_y; ybin++) {
        int    x     = min_x + xbin * step_x;
        int    y     = min_y + ybin * step_y;
        double value = (*histo)(chip, xbin, ybin);

        if (value != 0) {
          fprintf(fp, "%d %d->%d %d->%d %e\n", chip.chipId, xbin, x, ybin, y, value);
          h_eye.SetBinContent(xbin + 1, ybin + 1, value);
        }
      }
    }

    // calculate cumulative function along x (within [-yband, yband])
    std::vector<double> x_l, x_r;
    for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
      x_l.push_back((xbin != 0) ? x_l[xbin - 1] : 0.);
      x_r.push_back((xbin != 0) ? x_r[xbin - 1] : 0.);
      for (int ybin = nbin_y_half - yband; ybin < nbin_y_half + yband; ++ybin) {
        x_l.back() += (*histo)(chip, xbin, ybin);
        x_r.back() += (*histo)(chip, nbin_x - 1 - xbin, ybin);
      }
    }

    // calculate cumulative function along y (within [-xband, xband])
    std::vector<double> y_l, y_r;
    for (int ybin = 0; ybin < nbin_y_half; ++ybin) {
      y_l.push_back((ybin != 0) ? y_l[ybin - 1] : 0.);
      y_r.push_back((ybin != 0) ? y_r[ybin - 1] : 0.);
      for (int xbin = nbin_x_half - xband; xbin < nbin_x_half + xband; ++xbin) {
        y_l.back() += (*histo)(chip, xbin, ybin);
        y_r.back() += (*histo)(chip, xbin, nbin_y - 1 - ybin);
      }
    }

    // calculate opening as 85 % per-centile
    int open_l = -1;
    int open_r = -1;
    for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
      if ((open_l == -1) && ((x_l[xbin] / x_l.back()) > percent)) open_l = xbin;
      if ((open_r == -1) && ((x_r[xbin] / x_r.back()) > percent)) open_r = xbin;
    }

    int open_u = -1;
    int open_b = -1;
    for (int ybin = 0; ybin < nbin_y_half; ++ybin) {
      if ((open_b == -1) && ((y_l[ybin] / y_l.back()) > percent)) open_b = ybin;
      if ((open_u == -1) && ((y_r[ybin] / y_r.back()) > percent)) open_u = ybin;
    }

    // draw histogram
    h_eye.SetStats(kFALSE);
    h_eye.Draw("colz");
    c.Update();
    TLatex l;
    l.SetTextAlign(21);
    l.SetTextFont(43);
    l.SetTextSize(16);
    l.SetNDC(kTRUE);
    double open_x_l = min_x + open_l * step_x;
    double open_x_u = min_x + (nbin_x - 1 - open_r) * step_x;
    double x_center = min_x + nbin_x_half * step_x;
    double open_y_l = min_y + open_b * step_y;
    double open_y_u = min_y + (nbin_y - 1 - open_u) * step_y;
    double y_center = min_y + nbin_y_half * step_y;
    TArrow arrow_x(open_x_l, y_center, open_x_u, y_center, 0.05, "<->");
    TArrow arrow_y(x_center, open_y_l, x_center, open_y_u, 0.05, "<->");
    if ((open_l != -1) || (open_r != -1) || (open_b != -1) || (open_u != -1)) {
      l.DrawLatex(.5, .02,
                  TString::Format("openings (%g) in x [%g, %g], in y [%g, %g]", percent, open_x_l,
                                  open_x_u, open_y_l, open_y_u));
      arrow_x.Draw();
      arrow_y.Draw();
    }

    // write result to files
    std::string dirname = FindHicResultForChip(chip)->GetName();
    TDirectory *rootdir = rootfile_eye->GetDirectory(dirname.c_str());
    if (!rootdir) rootdir = rootfile_eye->mkdir(dirname.c_str());
    if (rootdir) rootdir->WriteTObject(&h_eye);
    if (h_eye.GetEntries() > 0) {
      c.SetLogz(kFALSE);
      c.Print(filename_eye.c_str());
      c.SetLogz(kTRUE);
      h_eye.SetMinimum(1.e-7);
      c.Print(filename_eye.c_str());
    }
  }
  c.Print((filename_eye + "]").c_str());
  fclose(fp);
  rootfile_eye->Close();
  std::cout << "Done" << std::endl;
}

void TEyeAnalysis::PlotHisto(TVirtualPad &p, TH2 &h, const std::string &filename)
{
  TVirtualPad *cPad = TVirtualPad::Pad();
  p.cd();

  // calculate cumulative function along x (within [-yband, yband])
  const int   nbin_x      = h.GetNbinsX();
  const int   nbin_x_half = nbin_x / 2;
  const int   nbin_y      = h.GetNbinsY();
  const int   nbin_y_half = nbin_y / 2;
  const int   xband       = 2;
  const int   yband       = 2;
  const float percent     = 0.95;

  std::vector<double> x_l, x_r;
  for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
    x_l.push_back((xbin != 0) ? x_l[xbin - 1] : 0.);
    x_r.push_back((xbin != 0) ? x_r[xbin - 1] : 0.);
    for (int ybin = nbin_y_half - yband; ybin < nbin_y_half + yband; ++ybin) {
      x_l.back() += h.GetBinContent(1 + xbin, 1 + ybin);
      x_r.back() += h.GetBinContent(1 + nbin_x - 1 - xbin, 1 + ybin);
    }
  }

  // calculate cumulative function along y (within [-xband, xband])
  std::vector<double> y_l, y_r;
  for (int ybin = 0; ybin < nbin_y_half; ++ybin) {
    y_l.push_back((ybin != 0) ? y_l[ybin - 1] : 0.);
    y_r.push_back((ybin != 0) ? y_r[ybin - 1] : 0.);
    for (int xbin = nbin_x_half - xband; xbin < nbin_x_half + xband; ++xbin) {
      y_l.back() += h.GetBinContent(1 + xbin, 1 + ybin);
      y_r.back() += h.GetBinContent(1 + xbin, 1 + nbin_y - 1 - ybin);
    }
  }

  // calculate opening as 85 % per-centile
  int open_l = -1;
  int open_r = -1;
  for (int xbin = 0; xbin < nbin_x_half; ++xbin) {
    if ((open_l == -1) && ((x_l[xbin] / x_l.back()) > percent)) open_l = xbin;
    if ((open_r == -1) && ((x_r[xbin] / x_r.back()) > percent)) open_r = xbin;
  }

  int open_u = -1;
  int open_b = -1;
  for (int ybin = 0; ybin < nbin_y_half; ++ybin) {
    if ((open_b == -1) && ((y_l[ybin] / y_l.back()) > percent)) open_b = ybin;
    if ((open_u == -1) && ((y_r[ybin] / y_r.back()) > percent)) open_u = ybin;
  }

  // draw histogram
  h.SetStats(kFALSE);
  h.Draw("colz");
  h.SetMinimum(1.e-7);
  p.SetLogz();
  p.Update();
  TLatex l;
  l.SetTextAlign(21);
  l.SetTextFont(43);
  l.SetTextSize(16);
  l.SetNDC(kTRUE);
  double open_x_l = h.GetXaxis()->GetBinCenter(1 + open_l);
  double open_x_u = h.GetXaxis()->GetBinCenter(1 + nbin_x - 1 - open_r);
  double x_center = h.GetXaxis()->GetBinCenter(1 + nbin_x_half);
  double open_y_l = h.GetYaxis()->GetBinCenter(1 + open_b);
  double open_y_u = h.GetYaxis()->GetBinCenter(1 + nbin_x - 1 - open_u);
  double y_center = h.GetYaxis()->GetBinCenter(1 + nbin_y_half);
  TArrow arrow_x(open_x_l, y_center, open_x_u, y_center, 0.05, "<->");
  TArrow arrow_y(x_center, open_y_l, x_center, open_y_u, 0.05, "<->");
  if ((open_l != -1) || (open_r != -1) || (open_b != -1) || (open_u != -1)) {
    l.DrawLatex(.5, .02,
                TString::Format("openings (%g) in x [%g, %g], in y [%g, %g]", percent, open_x_l,
                                open_x_u, open_y_l, open_y_u));
    arrow_x.Draw();
    arrow_y.Draw();
  }

  if (!filename.empty()) p.Print(filename.c_str());

  if (cPad) cPad->cd();
}
