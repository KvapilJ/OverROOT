#include "Data.h"

Data::Data(const TString &fileName):
    m_file(new TFile(fileName)),
    m_tree((TTree*)m_file->Get(GetTreeName())){

    m_dataNo = m_tree->GetListOfBranches()->GetSize();
    m_data = new Float_t[m_dataNo];
    m_dataName = new TString[m_dataNo];
    m_dataCut = new bool[m_dataNo];
    m_DataCutValue = new Float_t[2*m_dataNo];

    for(Int_t i=0; i<m_dataNo;i++){
        m_dataName[i]=m_tree->GetListOfBranches()->At(i)->GetName();
        m_tree->SetBranchAddress(m_dataName[i],&m_data[i]);
        m_dataCut[i] = false;
        m_DataCutValue[i] = -1;
        m_DataCutValue[i+m_dataNo] = -1;
    }
}

Data::~Data(){
    delete m_file;
    delete m_tree;
    delete [] m_data;
    delete [] m_dataName;
}

TString Data::GetTreeName(){
    TIter nextkey(m_file->GetListOfKeys());
    TKey *key;
    while((key=(TKey*)nextkey())){
        TObject *obj = key->ReadObj();
         if(obj->IsA()->InheritsFrom(TTree::Class())){
            return obj->GetName();
         }
    }
    return "";
}

TH1D *Data::Draw(const TString &name){
    return m_TH1D[GetHistoID(name)];
}

void Data::Fill(const TString &name, const TString &histname){
    Fill(name,histname,false);
}

void Data::Fill(const TString &name, const TString &histname, bool sumw){
    float min = 0;
    float max = 0;
    for (Int_t entry=0;entry<(Int_t)m_tree->GetEntries();entry++){
        m_tree->GetEntry(entry);
        min = m_data[GetVariableID(name)];
        max = m_data[GetVariableID(name)];
        if(m_data[GetVariableID(name)]<min) min = m_data[GetVariableID(name)];
        else max = m_data[GetVariableID(name)];
    }
    m_TH1D.push_back(new TH1D(histname,histname,constant.GetDefaultBinNumber(),min,max));
    m_TH1DName.push_back(histname);
    bool cut[m_dataNo];
    for (Int_t entry=0;entry<(Int_t)m_tree->GetEntries();entry++){
        m_tree->GetEntry(entry);
        for(Int_t i=0; i<m_dataNo;i++){
            if(m_dataCut[i]){
                if(m_DataCutValue[2*i] <= m_data[i] and m_data[i] <= m_DataCutValue[2*i+1])
                    cut[i] = true;
                else
                    cut[i] = false;
            }
            else
                cut[i] = true;
        }
        bool accept = true;
        for(Int_t i=0; i<m_dataNo;i++){
            accept*=cut[i];
        }
        if(accept)
            m_TH1D.back()->Fill(m_data[GetVariableID(name)]);
    }
    if(sumw)
        m_TH1D.back()->Sumw2();
}

void Data::GetVariableNames(){
    for(Int_t i=0; i<m_dataNo;i++){
        std::cout<<i<<": "<<m_dataName[i]<<std::endl;
    }
}

Int_t Data::GetVariableID(const TString &name){
    for(Int_t i=0; i<m_dataNo;i++){
        if(m_dataName[i]==name) return i;
    }
    return -1;
}

Int_t Data::GetHistoID(const TString &name){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1DName.begin(); it != m_TH1DName.end(); ++it){
        if(*it==name) return temp;
        temp++;
    }
    return -1;
}

void Data::SetCut(const TString &name, Float_t left, Float_t right){
    m_dataCut[GetVariableID(name)] = true;
    m_DataCutValue[2*GetVariableID(name)] = left;
    m_DataCutValue[2*GetVariableID(name)+1] = right;
}

void Data::ListCut(){
    for(Int_t i=0; i<m_dataNo;i++){
        cout<<std::boolalpha<<m_dataCut[i]<<" "<<m_dataName[i]<<" "<<m_DataCutValue[2*i]<<" "<<m_DataCutValue[2*i+1]<<endl;
    }
}

void Data::ListHisto(){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1DName.begin(); it != m_TH1DName.end(); ++it)
   // for( auto it_name : m_TH1DName )
    {
        cout<<temp<<": "<< *it <<endl;
        temp++;
    }
}

void Data::ListFits(){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1D_FitName.begin(); it != m_TH1D_FitName.end(); ++it){
        std::cout<<m_TH1D_FitName[temp]<<": "<< m_TH1D_FitValue[temp]<<"+-" <<m_TH1D_FitError[temp]<<std::endl;
        temp++;
    }
}

void Data::SetHistoTitle(const TString &name,const TString &title,const TString &xaxis,const TString &yaxis){
    m_TH1D[GetHistoID(name)]->SetTitle(title);
    m_TH1D[GetHistoID(name)]->GetXaxis()->SetTitle(xaxis);
    m_TH1D[GetHistoID(name)]->GetYaxis()->SetTitle(yaxis);
}

void Data::SetDraw(const TString &histname, Float_t xrangemin, Float_t xrangemax, Float_t yrangemin, Float_t yrangemax, Color_t color){
    m_TH1D[GetHistoID(histname)]->GetXaxis()->SetRangeUser(xrangemin,xrangemax);
    m_TH1D[GetHistoID(histname)]->GetYaxis()->SetRangeUser(yrangemin,yrangemax);
    m_TH1D[GetHistoID(histname)]->SetLineColor(color);
}

void Data::Fit(const TString &histname,const TString &function, Float_t left, Float_t right, Double_t *param){
    TF1 *tempfunc = new TF1(function,function,left,right);
    tempfunc->SetParameter(2,param[1]);
    tempfunc->SetParameter(3,param[2]);
    m_TH1D[GetHistoID(histname)]->Fit(tempfunc,"0R");

    for(Int_t i = 0; i < (m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetNpar());i++){
        m_TH1D_FitName.push_back(histname+" "+function+": "+m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetParName(i));
        m_TH1D_FitFunction.push_back(function);
        m_TH1D_FitValue.push_back(m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetParameter(i));
        m_TH1D_FitError.push_back(m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetParError(i));
    }
}

void Data::Fit(const TString &histname,const TString &function, Float_t left, Float_t right){
    TF1 *tempfunc = new TF1(function,function,left,right);
    //m_TH1D[GetHistoID(histname)]->Fit(function,"0","",left,right);
    m_TH1D[GetHistoID(histname)]->Fit(tempfunc,"0R");

    for(Int_t i = 0; i < (m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetNpar());i++){
        m_TH1D_FitName.push_back(histname+" "+function+": "+m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetParName(i));
        m_TH1D_FitFunction.push_back(function);
        m_TH1D_FitValue.push_back(m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetParameter(i));
        m_TH1D_FitError.push_back(m_TH1D[GetHistoID(histname)]->GetFunction(function)->GetParError(i));
    }

    //m_TH1D[GetHistoID(histname)]->Fit(function,"R","",1.7,2.1);
}

Double_t Data::GetFitValue(const TString &histname,const TString &paramname){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1D_FitName.begin(); it != m_TH1D_FitName.end(); ++it){
        if(it->Contains(histname) and it->Contains(paramname)) return m_TH1D_FitValue[temp];
        temp++;
    }
    return -1;
}

Double_t Data::GetFitError(const TString &histname,const TString &paramname){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1D_FitName.begin(); it != m_TH1D_FitName.end(); ++it){
        if(it->Contains(histname) and it->Contains(paramname)) return m_TH1D_FitError[temp];
        temp++;
    }
    return -1;
}

Double_t Data::FindScaleParamOffPeak(const TString &histnamefit, const TString &histname){
    Int_t binMin = m_TH1D[GetHistoID(histnamefit)]->GetXaxis()->FindBin(GetFitValue(histnamefit,"Mean")-3*GetFitValue(histnamefit,"Sigma"));
    Int_t binMax = m_TH1D[GetHistoID(histnamefit)]->GetXaxis()->FindBin(GetFitValue(histnamefit,"Mean")+3*GetFitValue(histnamefit,"Sigma"));
    Double_t first =  m_TH1D[GetHistoID(histnamefit)]->Integral(1,binMin)+m_TH1D[GetHistoID(histnamefit)]->Integral(binMax,m_TH1D[GetHistoID(histnamefit)]->GetSize()-2);
    Double_t second = m_TH1D[GetHistoID(histname)]->Integral(1,binMin)+m_TH1D[GetHistoID(histname)]->Integral(binMax,m_TH1D[GetHistoID(histname)]->GetSize()-2);
    return first/second;
}

void Data::Scale(const TString &histname, Double_t scale){
    m_TH1D[GetHistoID(histname)]->Scale(scale);
}

void Data::CorrectSignal(const TString &histsignal,const TString &histbackground, Float_t peakleft, Float_t peakright){
     Fit(histsignal,"gaus", peakleft,peakright);
     Scale(histbackground,FindScaleParamOffPeak(histsignal,histbackground));
     Clone(histsignal,histsignal+"_cor");
     m_TH1D[GetHistoID(histsignal+"_cor")]->GetListOfFunctions()->Delete();
     m_TH1D[GetHistoID(histsignal+"_cor")]->Add(m_TH1D[GetHistoID(histbackground)],-1);
     Double_t param[3];
     param[0] = GetFitValue(histsignal,"Constant");
     param[1] = GetFitValue(histsignal,"Mean");
     param[2] = GetFitValue(histsignal,"Sigma");
     Fit(histsignal+"_cor","gaus(0)+pol1(3)",1.7,2.1,param);
}

void Data::Clone(const TString &histname, const TString &histnamenew){
    m_TH1D.push_back((TH1D*)m_TH1D[GetHistoID(histname)]->Clone(histnamenew));
    m_TH1DName.push_back(histnamenew);
    m_TH1D[GetHistoID(histnamenew)]->SetTitle(histnamenew);
}

