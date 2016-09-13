#include "Data.h"

//constructor - open a file and link all branches in nuple like structure and init private variable
Data::Data(const TString &fileName):
    m_file(new TFile(fileName)), //open file
    m_tree((TTree*)m_file->Get(GetTreeName())){ //get TTree

    //init variable
    m_dataNo = m_tree->GetListOfBranches()->GetSize();
    m_data = new Float_t[m_dataNo];
    m_dataName = new TString[m_dataNo];
    m_dataCut = new Bool_t[m_dataNo];
    m_DataCutValue = new Float_t[2*m_dataNo];

    //link branch adress and init cuts
    for(Int_t i=0; i<m_dataNo;i++){
        m_dataName[i]=m_tree->GetListOfBranches()->At(i)->GetName();
        m_tree->SetBranchAddress(m_dataName[i],&m_data[i]);
        m_dataCut[i] = false;
        m_DataCutValue[i] = -1;
        m_DataCutValue[i+m_dataNo] = -1;
    }
}

//destructor - delete all dynamically created variable
Data::~Data(){
    delete m_file;
    delete m_tree;
    delete [] m_data;
    delete [] m_dataName;
}

//return tree name
TString Data::GetTreeName(){
    TIter nextkey(m_file->GetListOfKeys());
    TKey *key;
    while((key=(TKey*)nextkey())){ //iter over all elements in file and find TTree type
        TObject *obj = key->ReadObj();
         if(obj->IsA()->InheritsFrom(TTree::Class())){ //it is TTree?
            return obj->GetName(); //if yes - return name
         }
    }
    return ""; //otherwise return non
}

//return pointer to histogram at given name
TH1 *Data::Draw(const TString &name){
    return m_TH1D[GetHistoID(name)];
    //if(m_TH2D[GetHistoID(name)]) return m_TH2D[GetHistoID(name)];
    //cout<<m_TH1D[GetHistoID(name)];

}
/*
TH2D *Data::Draw2D(const TString &name){
    return m_TH2D[GetHistoID(name)];
}
*/
//fill 1D histogram with name histname with variable name without sumw
void Data::Fill(const TString &histname, const TString &name){
    Fill(histname,name,name,false,false);
}

//fill 1D histogram with name histname with variable name with/out sumw
void Data::Fill(const TString &histname, const TString &name, Int_t sumw){
    //cout<<"is a string: "<<sumw->InheritsFrom(TString::Class())<<endl;
    //cout<<"is a bool: "<<sumw->InheritsFrom(TDataType::Class())<<endl;
    //cout<<sumw<<endl;
    Fill(histname,name,name,false,sumw);
}

//fill 2D histogram with name histname with variable name without sumw
void Data::Fill(const TString &histname, const TString &name, const TString &name2){
    Fill(histname,name,name2,true,false);
}

//fill 2D histogram with name histname with variable name with/out sumw
void Data::Fill(const TString &histname, const TString &name, const TString &name2, Int_t sumw){
    Fill(histname,name,name2,true,sumw);
}

void Data::Fill(const TString &histname, const TString &name, const TString &name2,Bool_t twod, Int_t sumw){
    float min_x = 0;
    float max_x = 0;
    float min_y = 0;
    float max_y = 0;
    for (Int_t entry=0;entry<(Int_t)m_tree->GetEntries();entry++){ //loop over entries and find min and max
        m_tree->GetEntry(entry);
        min_x = m_data[GetVariableID(name)];
        max_x = m_data[GetVariableID(name)];
        if(m_data[GetVariableID(name)]<min_x) min_x = m_data[GetVariableID(name)];
        else max_x = m_data[GetVariableID(name)];
    }
    if(twod){
        for (Int_t entry=0;entry<(Int_t)m_tree->GetEntries();entry++){ //loop over entries and find min and max
            m_tree->GetEntry(entry);
            min_y = m_data[GetVariableID(name2)];
            max_y = m_data[GetVariableID(name2)];
            if(m_data[GetVariableID(name2)]<min_y) min_y = m_data[GetVariableID(name2)];
            else max_y = m_data[GetVariableID(name2)];
        }
        m_TH2D.push_back(new TH2D(histname,histname,constant.GetDefaultBinNumber(),min_x,max_x,constant.GetDefaultBinNumber(),min_y,max_y)); //create histogram with given name and default binnumber and fill it
        m_TH2DName.push_back(histname); //store histogram name
    }
    if(!twod){
        m_TH1D.push_back(new TH1D(histname,histname,constant.GetDefaultBinNumber(),min_x,max_x)); //create histogram with given name and default binnumber and fill it
        m_TH1DName.push_back(histname); //store histogram name
    }
    Bool_t cut[m_dataNo];
    for (Int_t entry=0;entry<(Int_t)m_tree->GetEntries();entry++){ //loop over entries
        m_tree->GetEntry(entry);
        for(Int_t i=0; i<m_dataNo;i++){ //loop over variable
            cut[i] = true; //if cut for that variable set
            if(m_dataCut[i] and (m_DataCutValue[2*i] > m_data[i] or m_data[i] > m_DataCutValue[2*i+1])){ //cut
                    cut[i] = false;
            }
        }
        Bool_t accept = true;
        for(Int_t i=0; i<m_dataNo;i++){ //loop over all cuts
            accept*=cut[i]; //true if variable passes all cuts
        }
        if(accept){ //if pass, fill histogram
            if(twod) m_TH2D.back()->Fill(m_data[GetVariableID(name)],m_data[GetVariableID(name2)]);
            if(!twod) m_TH1D.back()->Fill(m_data[GetVariableID(name)]);
        }
    }
    if(sumw ==1){ //if sumw need, sumw
        if(twod) m_TH2D.back()->Sumw2();
        else m_TH1D.back()->Sumw2();
    }
}

//print all variables
void Data::GetVariableNames(){
    for(Int_t i=0; i<m_dataNo;i++){
        std::cout<<i<<": "<<m_dataName[i]<<std::endl;
    }
}

//return variable ID
Int_t Data::GetVariableID(const TString &name){
    for(Int_t i=0; i<m_dataNo;i++){ //loop over variables
        if(m_dataName[i]==name) return i; //find variable with given name and return ID
    }
    return -1;
}

//return histogram ID
Int_t Data::GetHistoID(const TString &name){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1DName.begin(); it != m_TH1DName.end(); ++it){ //loop over histograms
        if(*it==name) return temp; //find histogram at given name and return ID
        temp++;
    }

    temp = 0;
    for(std::vector<TString>::iterator it = m_TH2DName.begin(); it != m_TH2DName.end(); ++it){ //loop over histograms
        if(*it==name) return temp; //find histogram at given name and return ID
        temp++;
    }
    return -1;
}

Int_t Data::GetFitID(const TString &name){
    Int_t temp = 0;
    for(std::vector<TF1*>::iterator it = m_TF1.begin(); it != m_TF1.end(); ++it){ //loop over fits
        if((*it)->GetName()==name) return temp; //find fit at given name and return ID
        temp++;
    }
    return -1;
}

//set interval cuts
void Data::SetCut(const TString &name, Float_t left, Float_t right){
    m_dataCut[GetVariableID(name)] = true; //activate cut
    m_DataCutValue[2*GetVariableID(name)] = left; //set minimum for cut
    m_DataCutValue[2*GetVariableID(name)+1] = right; //set maximum for cut
}

//list all cuts and its values
void Data::ListCut(){
    for(Int_t i=0; i<m_dataNo;i++){
        cout<<std::boolalpha<<m_dataCut[i]<<" "<<m_dataName[i]<<" "<<m_DataCutValue[2*i]<<" "<<m_DataCutValue[2*i+1]<<endl;
    }
}

//list all stored histograms ID and names
void Data::ListHisto(){
    Int_t temp = 0;
    for(std::vector<TString>::iterator it = m_TH1DName.begin(); it != m_TH1DName.end(); ++it)
   // for( auto it_name : m_TH1DName )
    {
        cout<<temp<<": "<< *it <<endl;
        temp++;
    }
}

//list all stored fits
void Data::ListFits(){
    for(std::vector<TF1*>::iterator it = m_TF1.begin(); it != m_TF1.end(); ++it){ //loop over fits
        cout<<(*it)->GetName()<<":"<<endl;
        for(Int_t par = 0; par < (*it)->GetNpar();par++){
            cout<<"    "<<(*it)->GetParName(par)<<": "<<(*it)->GetParameter(par)<<"+-"<<(*it)->GetParError(par)<<endl;
        }
    }
}

//set histogram title and axis names
void Data::SetHistoTitle(const TString &name,const TString &title,const TString &xaxis,const TString &yaxis){
    m_TH1D[GetHistoID(name)]->SetTitle(title);
    m_TH1D[GetHistoID(name)]->GetXaxis()->SetTitle(xaxis);
    m_TH1D[GetHistoID(name)]->GetYaxis()->SetTitle(yaxis);
}

//set histogram ranges and colors
void Data::SetDraw(const TString &histname, Float_t xrangemin, Float_t xrangemax, Float_t yrangemin, Float_t yrangemax, Color_t color){
    m_TH1D[GetHistoID(histname)]->GetXaxis()->SetRangeUser(xrangemin,xrangemax);
    m_TH1D[GetHistoID(histname)]->GetYaxis()->SetRangeUser(yrangemin,yrangemax);
    m_TH1D[GetHistoID(histname)]->SetLineColor(color);
}

//fit histogram with function in left-right range with given parameters, that can be fixed
void Data::Fit(const TString &histname,const TString &function, Float_t left, Float_t right, Double_t *param, std::vector<Bool_t> fix){
    m_TF1.push_back(new TF1(histname+":"+function,function,left,right)); //create fit function
    Int_t fix_pos = 0;
    TString fitoption;
    if(param != NULL){
        for(std::vector<Bool_t>::iterator it = fix.begin(); it != fix.end(); ++it){//loop over parameters
            if(*it) m_TF1[GetFitID(histname+":"+function)]->SetParameter(fix_pos,param[fix_pos]); //fix them if required
            fix_pos++;
        }
        fitoption = "B0R"; //set option to fixed param and range
    }
    else
        fitoption = "0R"; //otherwise fix only range

    m_TH1D[GetHistoID(histname)]->Fit(m_TF1[GetFitID(histname+":"+function)],fitoption); //fit histogram
}

//fit histogram with fiven function in range left-right
void Data::Fit(const TString &histname,const TString &function, Float_t left, Float_t right){
    std::vector<Bool_t> zero;
    zero.push_back(0);
    Fit(histname,function,left,right,NULL,zero);
}

//return parameter value of name paramname in histogram histname
Double_t Data::GetFitValue(const TString &histname, const TString &function,const TString &paramname){
    for(std::vector<TF1*>::iterator it = m_TF1.begin(); it != m_TF1.end(); ++it){
        if((*it)->GetName() == (histname+":"+function)) return (*it)->GetParameter(paramname);
    }
    return -1;
}

//return parameter value error of name paramname in histogram histname
Double_t Data::GetFitError(const TString &histname, const TString &function,const TString &paramname){
    for(std::vector<TF1*>::iterator it = m_TF1.begin(); it != m_TF1.end(); ++it){
        if((*it)->GetName() == (histname+":"+function)) return (*it)->GetParError((*it)->GetParNumber(paramname));
    }
    return -1;
}

//HISTOGRAM NEED TO BE FITTED WITH GAUSSIAN!!!
//histnamefit signal with backgroud fitted with gaussian, histname - backgroud
//calculate all entries in S+B and B off 3-sigma gaussian peak
Double_t Data::FindScaleParamOffPeak(const TString &histsignal, const TString &histbackground){
    Int_t binMin = m_TH1D[GetHistoID(histsignal)]->GetXaxis()->FindBin(GetFitValue(histsignal,"gaus","Mean")-3*GetFitValue(histsignal,"gaus","Sigma"));
    Int_t binMax = m_TH1D[GetHistoID(histsignal)]->GetXaxis()->FindBin(GetFitValue(histsignal,"gaus","Mean")+3*GetFitValue(histsignal,"gaus","Sigma"));
    Double_t first =  m_TH1D[GetHistoID(histsignal)]->Integral(1,binMin)+m_TH1D[GetHistoID(histsignal)]->Integral(binMax,m_TH1D[GetHistoID(histsignal)]->GetSize()-2);
    Double_t second = m_TH1D[GetHistoID(histbackground)]->Integral(1,binMin)+m_TH1D[GetHistoID(histbackground)]->Integral(binMax,m_TH1D[GetHistoID(histbackground)]->GetSize()-2);
    return first/second;
}

//scale given histogram with scaling param scale
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
     param[0] = GetFitValue(histsignal,"gaus","Constant");
     param[1] = GetFitValue(histsignal,"gaus","Mean");
     param[2] = GetFitValue(histsignal,"gaus","Sigma");
     std::vector<Bool_t> b;
     b.push_back(0);b.push_back(1);b.push_back(1);
     Fit(histsignal+"_cor","gaus(0)+pol1(3)",1.7,2.1,param,b);
     param[0] = GetFitValue(histsignal+"_cor","gaus(0)+pol1(3)","p3");
     param[1] = GetFitValue(histsignal+"_cor","gaus(0)+pol1(3)","p4");
     TF1 *rezBG = new TF1("rezBG","[0]+[1]*x",1.7,2.1);
     rezBG->SetParameter(0,param[0]);
     rezBG->SetParameter(1,param[1]);
     //std::vector<bool> a;
     //a.push_back(1);a.push_back(1);
     //Fit(histsignal+"_cor","pol1(0)",1.7,2.1,param,a);
     Clone(histsignal+"_cor",histsignal+"_cor_rez");
     m_TH1D[GetHistoID(histsignal+"_cor_rez")]->GetListOfFunctions()->Delete();
     m_TH1D[GetHistoID(histsignal+"_cor_rez")]->Add(rezBG,-1);
     param[0] = GetFitValue(histsignal,"gaus","Constant");
     param[1] = GetFitValue(histsignal,"gaus","Mean");
     param[2] = GetFitValue(histsignal,"gaus","Sigma");
     Fit(histsignal+"_cor_rez","gaus(0)+pol1(3)",1.7,2.1,param,b);
     ListFits();
}


//clone given histogram to a new histogram
void Data::Clone(const TString &histname, const TString &histnamenew){
    m_TH1D.push_back((TH1D*)m_TH1D[GetHistoID(histname)]->Clone(histnamenew));
    m_TH1DName.push_back(histnamenew);
    m_TH1D[GetHistoID(histnamenew)]->SetTitle(histnamenew);
}

