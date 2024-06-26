{
//========= Macro generated from object: /
//========= by ROOT version6.26/04
   
   Double_t _fx1005[45] = {
   15,
   16,
   17,
   18,
   19,
   20,
   21,
   22,
   23,
   24,
   25,
   26,
   27,
   28,
   29,
   30,
   31,
   32,
   33,
   34,
   35,
   36,
   37,
   38,
   39,
   40,
   41,
   42,
   43,
   44,
   45,
   46,
   47,
   48,
   49,
   50,
   51,
   52,
   53,
   54,
   55,
   56,
   57,
   58,
   59};
   Double_t _fy1005[45] = {
   0.993879,
   0.991594,
   0.988668,
   0.984499,
   0.979145,
   0.97233,
   0.963816,
   0.952452,
   0.938565,
   0.921635,
   0.900035,
   0.873875,
   0.842538,
   0.804438,
   0.761217,
   0.711872,
   0.655285,
   0.594251,
   0.529842,
   0.463151,
   0.396418,
   0.332283,
   0.268548,
   0.214373,
   0.162869,
   0.12012,
   0.0859669,
   0.0579399,
   0.0394005,
   0.0250717,
   0.01511,
   0.00898972,
   0.00517921,
   0.0027462,
   0.00148697,
   0.000775528,
   0.000395966,
   0.000182343,
   8.88824e-05,
   4.69208e-05,
   2.28882e-05,
   1.25885e-05,
   8.7738e-06,
   8.01086e-06,
   5.34058e-06};
   Double_t _fex1005[45] = {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0};
   Double_t _fey1005[45] = {
   0.0348826,
   0.040829,
   0.0473361,
   0.0552469,
   0.0639056,
   0.0733543,
   0.083516,
   0.0951701,
   0.107388,
   0.120187,
   0.134143,
   0.148471,
   0.162891,
   0.177379,
   0.190665,
   0.202539,
   0.21255,
   0.219598,
   0.223208,
   0.222999,
   0.218756,
   0.210652,
   0.198207,
   0.18353,
   0.165132,
   0.14539,
   0.125361,
   0.104482,
   0.0870035,
   0.0699187,
   0.0545558,
   0.0422111,
   0.0321011,
   0.0234037,
   0.0172323,
   0.0124493,
   0.00889729,
   0.00603837,
   0.00421603,
   0.00306328,
   0.00213952,
   0.00158672,
   0.00132467,
   0.00126576,
   0.00103349};
   gre = new TGraphErrors(45,_fx1005,_fy1005,_fex1005,_fey1005);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);
   
   TH1F *Graph_Graph1005 = new TH1F("Graph_Graph1005","",100,10.6,63.4);
   Graph_Graph1005->SetMinimum(-0.157126);
   Graph_Graph1005->SetMaximum(1.15715);
   Graph_Graph1005->SetDirectory(0);
   Graph_Graph1005->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph1005->SetLineColor(ci);
   Graph_Graph1005->GetXaxis()->SetLabelFont(42);
   Graph_Graph1005->GetXaxis()->SetTitleOffset(1);
   Graph_Graph1005->GetXaxis()->SetTitleFont(42);
   Graph_Graph1005->GetYaxis()->SetLabelFont(42);
   Graph_Graph1005->GetYaxis()->SetTitleFont(42);
   Graph_Graph1005->GetZaxis()->SetLabelFont(42);
   Graph_Graph1005->GetZaxis()->SetTitleOffset(1);
   Graph_Graph1005->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1005);
   
   
   TF1 *fitFunc1006 = new TF1("*fitFunc",10.6,63.4,2);
    //The original function : fitFunc had originally been created by:
    //TF1 *fitFunc = new TF1("fitFunc",fitFunc,10.6,63.4,2, 1, TF1::EAddToList::kNo);
   fitFunc1006->SetRange(10.6,63.4);
   fitFunc1006->SetName("fitFunc");
   fitFunc1006->SetTitle("fitFunc");
   fitFunc1006->SetSavedPoint(0,0.999923);
   fitFunc1006->SetSavedPoint(1,0.999891);
   fitFunc1006->SetSavedPoint(2,0.999845);
   fitFunc1006->SetSavedPoint(3,0.999783);
   fitFunc1006->SetSavedPoint(4,0.999697);
   fitFunc1006->SetSavedPoint(5,0.999581);
   fitFunc1006->SetSavedPoint(6,0.999424);
   fitFunc1006->SetSavedPoint(7,0.999215);
   fitFunc1006->SetSavedPoint(8,0.998938);
   fitFunc1006->SetSavedPoint(9,0.998573);
   fitFunc1006->SetSavedPoint(10,0.998097);
   fitFunc1006->SetSavedPoint(11,0.997482);
   fitFunc1006->SetSavedPoint(12,0.996691);
   fitFunc1006->SetSavedPoint(13,0.995684);
   fitFunc1006->SetSavedPoint(14,0.994411);
   fitFunc1006->SetSavedPoint(15,0.992816);
   fitFunc1006->SetSavedPoint(16,0.990831);
   fitFunc1006->SetSavedPoint(17,0.988382);
   fitFunc1006->SetSavedPoint(18,0.985384);
   fitFunc1006->SetSavedPoint(19,0.981743);
   fitFunc1006->SetSavedPoint(20,0.977356);
   fitFunc1006->SetSavedPoint(21,0.972112);
   fitFunc1006->SetSavedPoint(22,0.965894);
   fitFunc1006->SetSavedPoint(23,0.958578);
   fitFunc1006->SetSavedPoint(24,0.950038);
   fitFunc1006->SetSavedPoint(25,0.94015);
   fitFunc1006->SetSavedPoint(26,0.928792);
   fitFunc1006->SetSavedPoint(27,0.915846);
   fitFunc1006->SetSavedPoint(28,0.90121);
   fitFunc1006->SetSavedPoint(29,0.884792);
   fitFunc1006->SetSavedPoint(30,0.866522);
   fitFunc1006->SetSavedPoint(31,0.846352);
   fitFunc1006->SetSavedPoint(32,0.824261);
   fitFunc1006->SetSavedPoint(33,0.800256);
   fitFunc1006->SetSavedPoint(34,0.77438);
   fitFunc1006->SetSavedPoint(35,0.746707);
   fitFunc1006->SetSavedPoint(36,0.717346);
   fitFunc1006->SetSavedPoint(37,0.686441);
   fitFunc1006->SetSavedPoint(38,0.654169);
   fitFunc1006->SetSavedPoint(39,0.620735);
   fitFunc1006->SetSavedPoint(40,0.586373);
   fitFunc1006->SetSavedPoint(41,0.551336);
   fitFunc1006->SetSavedPoint(42,0.515894);
   fitFunc1006->SetSavedPoint(43,0.480325);
   fitFunc1006->SetSavedPoint(44,0.444913);
   fitFunc1006->SetSavedPoint(45,0.409935);
   fitFunc1006->SetSavedPoint(46,0.37566);
   fitFunc1006->SetSavedPoint(47,0.342339);
   fitFunc1006->SetSavedPoint(48,0.310203);
   fitFunc1006->SetSavedPoint(49,0.279454);
   fitFunc1006->SetSavedPoint(50,0.250267);
   fitFunc1006->SetSavedPoint(51,0.22278);
   fitFunc1006->SetSavedPoint(52,0.1971);
   fitFunc1006->SetSavedPoint(53,0.173298);
   fitFunc1006->SetSavedPoint(54,0.151411);
   fitFunc1006->SetSavedPoint(55,0.131445);
   fitFunc1006->SetSavedPoint(56,0.113374);
   fitFunc1006->SetSavedPoint(57,0.0971499);
   fitFunc1006->SetSavedPoint(58,0.0826979);
   fitFunc1006->SetSavedPoint(59,0.0699265);
   fitFunc1006->SetSavedPoint(60,0.0587298);
   fitFunc1006->SetSavedPoint(61,0.0489913);
   fitFunc1006->SetSavedPoint(62,0.0405882);
   fitFunc1006->SetSavedPoint(63,0.0333947);
   fitFunc1006->SetSavedPoint(64,0.0272855);
   fitFunc1006->SetSavedPoint(65,0.0221382);
   fitFunc1006->SetSavedPoint(66,0.0178358);
   fitFunc1006->SetSavedPoint(67,0.014268);
   fitFunc1006->SetSavedPoint(68,0.0113327);
   fitFunc1006->SetSavedPoint(69,0.0089371);
   fitFunc1006->SetSavedPoint(70,0.00699731);
   fitFunc1006->SetSavedPoint(71,0.00543907);
   fitFunc1006->SetSavedPoint(72,0.00419725);
   fitFunc1006->SetSavedPoint(73,0.00321542);
   fitFunc1006->SetSavedPoint(74,0.00244531);
   fitFunc1006->SetSavedPoint(75,0.00184603);
   fitFunc1006->SetSavedPoint(76,0.00138339);
   fitFunc1006->SetSavedPoint(77,0.00102906);
   fitFunc1006->SetSavedPoint(78,0.000759832);
   fitFunc1006->SetSavedPoint(79,0.000556885);
   fitFunc1006->SetSavedPoint(80,0.000405114);
   fitFunc1006->SetSavedPoint(81,0.000292512);
   fitFunc1006->SetSavedPoint(82,0.000209631);
   fitFunc1006->SetSavedPoint(83,0.000149111);
   fitFunc1006->SetSavedPoint(84,0.000105268);
   fitFunc1006->SetSavedPoint(85,7.37579e-05);
   fitFunc1006->SetSavedPoint(86,5.1291e-05);
   fitFunc1006->SetSavedPoint(87,3.53987e-05);
   fitFunc1006->SetSavedPoint(88,2.42461e-05);
   fitFunc1006->SetSavedPoint(89,1.64816e-05);
   fitFunc1006->SetSavedPoint(90,1.11187e-05);
   fitFunc1006->SetSavedPoint(91,7.44396e-06);
   fitFunc1006->SetSavedPoint(92,4.94585e-06);
   fitFunc1006->SetSavedPoint(93,3.26107e-06);
   fitFunc1006->SetSavedPoint(94,2.13383e-06);
   fitFunc1006->SetSavedPoint(95,1.38558e-06);
   fitFunc1006->SetSavedPoint(96,8.92849e-07);
   fitFunc1006->SetSavedPoint(97,5.70939e-07);
   fitFunc1006->SetSavedPoint(98,3.62298e-07);
   fitFunc1006->SetSavedPoint(99,2.2814e-07);
   fitFunc1006->SetSavedPoint(100,1.42559e-07);
   fitFunc1006->SetSavedPoint(101,10.6);
   fitFunc1006->SetSavedPoint(102,63.4);
   fitFunc1006->SetFillColor(19);
   fitFunc1006->SetFillStyle(0);
   fitFunc1006->SetLineColor(2);
   fitFunc1006->SetLineWidth(2);
   fitFunc1006->SetChisquare(0.524604);
   fitFunc1006->SetNDF(43);
   fitFunc1006->GetXaxis()->SetLabelFont(42);
   fitFunc1006->GetXaxis()->SetTitleOffset(1);
   fitFunc1006->GetXaxis()->SetTitleFont(42);
   fitFunc1006->GetYaxis()->SetLabelFont(42);
   fitFunc1006->GetYaxis()->SetTitleFont(42);
   fitFunc1006->SetParameter(0,33.0119);
   fitFunc1006->SetParError(0,0.844401);
   fitFunc1006->SetParLimits(0,0,0);
   fitFunc1006->SetParameter(1,5.92011);
   fitFunc1006->SetParError(1,0.715779);
   fitFunc1006->SetParLimits(1,0,0);
   fitFunc1006->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitFunc1006);
   gre->Draw("");
}
