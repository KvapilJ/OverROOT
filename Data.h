#include <iostream>
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TDataType.h"
#include "TCanvas.h"
#include "TString.h"
#include "TCollection.h"
#include "TKey.h"
#include "TObject.h"
#include "TColor.h"
#include "Canvas.h"
#include "Constants.h"
#ifndef DATA_H
#define DATA_H

class Data {
public:
    Data(const TString &fileName);
    ~Data();
    TString GetTreeName();
    TH1* Draw(const TString &name);
    void GetVariableNames();
    Int_t GetVariableID(const TString &name);
    Int_t GetHistoID(const TString &name);
    Int_t GetFitID(const TString &name);
    void SetCut(const TString &name, Float_t left, Float_t right);
    void ListHisto();
    void ListCut();
    void ListFits();
    void Fill(const TString &histname, const TString &name);
    void Fill(const TString &histname, const TString &name, const TString &name2);
    void Fill(const TString &histname, const TString &name, Int_t sumw);

    void Fill(const TString &histname, const TString &name, const TString &name2,Int_t sumw);
    void Fill(const TString &histname, const TString &name, const TString &name2,Bool_t twod,Int_t sumw);
    void SetHistoTitle(const TString &name,const TString &title,const TString &xaxis,const TString &yaxis);
    void SetDraw(const TString &histname, Float_t xrangemin, Float_t xrangemax, Float_t yrangemin, Float_t yrangemax, Color_t color);
    void Fit(const TString &histname,const TString &function, Float_t left, Float_t right);
    void Fit(const TString &histname,const TString &function, Float_t left, Float_t right, Double_t *param, std::vector<Bool_t> fix);
    Double_t GetFitValue(const TString &histname, const TString &function,const TString &paramname);
    Double_t GetFitError(const TString &histname, const TString &function, const TString &paramname);
    Double_t FindScaleParamOffPeak(const TString &histsignal, const TString &histbackground);
    void Scale(const TString &histname, Double_t scale);
    void CorrectSignal(const TString &histsignal,const TString &histbackground, Float_t peakleft, Float_t peakright);
    void Clone(const TString &histname, const TString &histnamenew);

private:
    TFile *m_file;
    TTree *m_tree;
    Int_t m_dataNo;
    Float_t *m_data;
    TString *m_dataName;
    Bool_t *m_dataCut;
    Float_t *m_DataCutValue;
    std::vector<TH1D*> m_TH1D;
    std::vector<TString> m_TH1DName;
    std::vector<TH2D*> m_TH2D;
    std::vector<TString> m_TH2DName;
    std::vector<TF1*> m_TF1;
};

#endif
