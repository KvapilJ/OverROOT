#include <iostream>
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TString.h"
#include "TCollection.h"
#include "TKey.h"
#include "TF1.h"
#include "TObject.h"
#include "Constants.h"
#ifndef CANVAS_H
#define CANVAS_H

class Canvas {
public:
    Canvas();
    ~Canvas();
    void Create(const TString &name);
    void Create(const TString &name, Int_t nx, Int_t ny);
    int GetCanvasID(const TString &name);
    void Put(const TString &name, TH1D *th1);
    void Put(const TString &name, TH1D *th1, Int_t position);
    void Flush(const TString &type);

private:
    std::vector<TCanvas*> m_canvas;
    std::vector<TString> m_canvasName;
    std::vector<Int_t> m_nHistoPerCanvas;
    Int_t m_numberOfCanvas;

};

Canvas::Canvas(){
    m_numberOfCanvas = 0;
}

void Canvas::Create(const TString &name){
    Create(name,1,1);
}

void Canvas::Create(const TString &name, Int_t nx, Int_t ny){
    m_canvas.push_back(new TCanvas(name,name,nx*constant.GetDefaultCanvasSize("x"),ny*constant.GetDefaultCanvasSize("y")));
    m_canvasName.push_back(name);
    m_nHistoPerCanvas.push_back(0);
    m_canvas[GetCanvasID(name)]->Divide(nx,ny);
}

int Canvas::GetCanvasID(const TString &name){
    Int_t position = 0;
    for(std::vector<TString>::iterator it = m_canvasName.begin(); it != m_canvasName.end(); ++it){
    if(*it == name) return position;
    else position++;
    }
    return -1;
}

void Canvas::Put(const TString &name, TH1D *th1){
    m_canvas[GetCanvasID(name)]->cd(m_nHistoPerCanvas[GetCanvasID(name)]+1);
    th1->Draw();
    //th1->GetListOfFunctions()->Print();
    //cout<<th1->GetListOfFunctions()->Last()->GetName();
    //cout<<"is: "<<th1->GetListOfFunctions()->Last()->IsA()->InheritsFrom(TF1::Class());
    //th1->GetFunction(th1->GetListOfFunctions()->Last()->GetName())->ResetBit(1<<9);
    TIter next(th1->GetListOfFunctions());
    TObject* object = 0;
    while ((object = next())){
        if(object->InheritsFrom(TF1::Class()))
            th1->GetFunction(object->GetName())->ResetBit(1<<9);
    }
    m_nHistoPerCanvas[GetCanvasID(name)]++;
}

void Canvas::Put(const TString &name, TH1D *th1, Int_t position){
    m_canvas[GetCanvasID(name)]->cd(position);
    th1->Draw("same");
}

void Canvas::Flush(const TString &type){
    Int_t position = 0;
    for(std::vector<TCanvas*>::iterator it = m_canvas.begin(); it != m_canvas.end(); ++it){
        m_canvas[position]->SaveAs(m_canvasName[position]+"."+type);
        position++;
    }
}

#endif
