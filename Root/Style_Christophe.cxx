#include "TStyle.h"


void Style_Christophe()
{
   TStyle *tmpStyle = new TStyle("Christophe", "Plain Style (no colors/fill areas)");

   tmpStyle->SetNdivisions(510, "x");
   tmpStyle->SetNdivisions(510, "y");
   tmpStyle->SetNdivisions(510, "z");
   tmpStyle->SetAxisColor(1, "x");
   tmpStyle->SetAxisColor(1, "y");
   tmpStyle->SetAxisColor(1, "z");
   tmpStyle->SetLabelColor(1, "x");
   tmpStyle->SetLabelColor(1, "y");
   tmpStyle->SetLabelColor(1, "z");
   tmpStyle->SetLabelFont(62, "x");
   tmpStyle->SetLabelFont(62, "y");
   tmpStyle->SetLabelFont(62, "z");
   tmpStyle->SetLabelOffset(0.005, "x");
   tmpStyle->SetLabelOffset(0.005, "y");
   tmpStyle->SetLabelOffset(0.005, "z");
   tmpStyle->SetLabelSize(0.04, "x");
   tmpStyle->SetLabelSize(0.04, "y");
   tmpStyle->SetLabelSize(0.04, "z");
   tmpStyle->SetTickLength(0.03, "x");
   tmpStyle->SetTickLength(0.03, "y");
   tmpStyle->SetTickLength(0.03, "z");
   tmpStyle->SetTitleOffset(0.9, "x");
   tmpStyle->SetTitleOffset(1.1, "y");
   tmpStyle->SetTitleOffset(1, "z");
   tmpStyle->SetTitleSize(0.05, "x");
   tmpStyle->SetTitleSize(0.05, "y");
   tmpStyle->SetTitleSize(0.05, "z");
   tmpStyle->SetTitleColor(1, "x");
   tmpStyle->SetTitleColor(1, "y");
   tmpStyle->SetTitleColor(1, "z");
   tmpStyle->SetTitleFont(72, "x");
   tmpStyle->SetTitleFont(72, "y");
   tmpStyle->SetTitleFont(72, "z");
   tmpStyle->SetBarWidth(1);
   tmpStyle->SetBarOffset(0);
   tmpStyle->SetDrawBorder(0);
   tmpStyle->SetOptLogx(0);
   tmpStyle->SetOptLogy(0);
   tmpStyle->SetOptLogz(0);
   tmpStyle->SetOptDate(0);
   tmpStyle->SetOptStat(0);
   tmpStyle->SetOptTitle(0);
   tmpStyle->SetOptFit(0);
   tmpStyle->SetNumberContours(20);
   tmpStyle->GetAttDate()->SetTextFont(62);
   tmpStyle->GetAttDate()->SetTextSize(0.025);
   tmpStyle->GetAttDate()->SetTextAngle(0);
   tmpStyle->GetAttDate()->SetTextAlign(11);
   tmpStyle->GetAttDate()->SetTextColor(1);
   tmpStyle->SetDateX(0.01);
   tmpStyle->SetDateY(0.01);
   tmpStyle->SetEndErrorSize(2);
   tmpStyle->SetErrorX(0.5);
   tmpStyle->SetFuncColor(2);
   tmpStyle->SetFuncStyle(1);
   tmpStyle->SetFuncWidth(3);
   tmpStyle->SetGridColor(0);
   tmpStyle->SetGridStyle(3);
   tmpStyle->SetGridWidth(1);
   tmpStyle->SetLegendBorderSize(1);
   tmpStyle->SetLegendFillColor(0);
   tmpStyle->SetLegendFont(0);
   tmpStyle->SetHatchesLineWidth(1);
   tmpStyle->SetHatchesSpacing(1);
   tmpStyle->SetFrameFillColor(0);
   tmpStyle->SetFrameLineColor(1);
   tmpStyle->SetFrameFillStyle(1001);
   tmpStyle->SetFrameLineStyle(1);
   tmpStyle->SetFrameLineWidth(1);
   tmpStyle->SetFrameBorderSize(1);
   tmpStyle->SetFrameBorderMode(0);
   tmpStyle->SetHistFillColor(0);
   tmpStyle->SetHistLineColor(1);
   tmpStyle->SetHistFillStyle(1001);
   tmpStyle->SetHistLineStyle(1);
   tmpStyle->SetHistLineWidth(2);
   tmpStyle->SetHistMinimumZero(kFALSE);
   tmpStyle->SetCanvasPreferGL(kFALSE);
   tmpStyle->SetCanvasColor(0);
   tmpStyle->SetCanvasBorderSize(2);
   tmpStyle->SetCanvasBorderMode(0);
   tmpStyle->SetCanvasDefH(600);
   tmpStyle->SetCanvasDefW(800);
   tmpStyle->SetCanvasDefX(10);
   tmpStyle->SetCanvasDefY(10);
   tmpStyle->SetPadColor(0);
   tmpStyle->SetPadBorderSize(2);
   tmpStyle->SetPadBorderMode(0);
   tmpStyle->SetPadBottomMargin(0.1);
   tmpStyle->SetPadTopMargin(0.05);
   tmpStyle->SetPadLeftMargin(0.1);
   tmpStyle->SetPadRightMargin(0.01);
   tmpStyle->SetPadGridX(kFALSE);
   tmpStyle->SetPadGridY(kFALSE);
   tmpStyle->SetPadTickX(0);
   tmpStyle->SetPadTickY(0);
   tmpStyle->SetPaperSize(20, 26);
   tmpStyle->SetScreenFactor(1);
   tmpStyle->SetStatColor(0);
   tmpStyle->SetStatTextColor(1);
   tmpStyle->SetStatBorderSize(1);
   tmpStyle->SetStatFont(62);
   tmpStyle->SetStatFontSize(0);
   tmpStyle->SetStatStyle(1001);
   tmpStyle->SetStatFormat("6.4g");
   tmpStyle->SetStatX(0.98);
   tmpStyle->SetStatY(0.995);
   tmpStyle->SetStatW(0.2);
   tmpStyle->SetStatH(0.16);
   tmpStyle->SetStripDecimals(kTRUE);
   tmpStyle->SetTitleAlign(13);
   tmpStyle->SetTitleFillColor(0);
   tmpStyle->SetTitleTextColor(1);
   tmpStyle->SetTitleBorderSize(0);
   tmpStyle->SetTitleFont(72);
   tmpStyle->SetTitleFontSize(0);
   tmpStyle->SetTitleStyle(1001);
   tmpStyle->SetTitleX(0.01);
   tmpStyle->SetTitleY(0.995);
   tmpStyle->SetTitleW(0);
   tmpStyle->SetTitleH(0);
   tmpStyle->SetLegoInnerR(0.5);

   Int_t fPaletteColor[50] = {51, 52, 53, 54, 55, 56, 57, 58, 59, 
                             60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 
                             70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
                             80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 
                             90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
   tmpStyle->SetPalette(50, fPaletteColor);

   TString fLineStyleArrayTmp[30] = {"", "  ", " 12 12", " 4 8", 
                             " 12 16 4 16", " 20 12 4 12", " 20 12 4 12 4 12 4 12", " 20 20", " 20 12 4 12 4 12", 
                             " 80 20", " 80 40 4 40", "  ", "  ", "  ", 
                             "  ", "  ", "  ", "  ", "  ", 
                             "  ", "  ", "  ", "  ", "  ", 
                             "  ", "  ", "  ", "  ", "  ", "  "};
   for (Int_t i=0; i<30; i++)
      tmpStyle->SetLineStyleString(i, fLineStyleArrayTmp[i]);

   tmpStyle->SetHeaderPS("");
   tmpStyle->SetTitlePS("");
   tmpStyle->SetFitFormat("5.4g");
   tmpStyle->SetPaintTextFormat("g");
   tmpStyle->SetLineScalePS(3);
   tmpStyle->SetColorModelPS(0);
   tmpStyle->SetTimeOffset(788918400);

   tmpStyle->SetLineColor(1);
   tmpStyle->SetLineStyle(1);
   tmpStyle->SetLineWidth(1);
   tmpStyle->SetFillColor(19);
   tmpStyle->SetFillStyle(1001);
   tmpStyle->SetMarkerColor(1);
   tmpStyle->SetMarkerSize(1);
   tmpStyle->SetMarkerStyle(1);
   tmpStyle->SetTextAlign(11);
   tmpStyle->SetTextAngle(0);
   tmpStyle->SetTextColor(1);
   tmpStyle->SetTextFont(72);
   tmpStyle->SetTextSize(0.05);

   tmpStyle->cd();
}
