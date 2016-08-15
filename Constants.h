#include "TString.h"
#ifndef CONSTANTS_H
#define CONSTANTS_H

class Constants {
public:
    Constants();
    ~Constants();
    void SetDefaultBinNumber(Int_t bin){m_default_TH1D_bin = bin;}
    void SetDefaultCanvasSize(Int_t x,Int_t y){m_default_Canvas_XY[0]=x;m_default_Canvas_XY[1]=y;}
    Int_t GetDefaultBinNumber(){return m_default_TH1D_bin;}
    Int_t GetDefaultCanvasSize(const TString &coordinate);
private:
    Int_t m_default_TH1D_bin;
    Int_t *m_default_Canvas_XY;

};
Constants constant;

Constants::Constants(){
    m_default_TH1D_bin=100;
    m_default_Canvas_XY = new Int_t[2];
    m_default_Canvas_XY[0]=800;
    m_default_Canvas_XY[1]=800;
}

Constants::~Constants(){
    delete m_default_Canvas_XY;
}



Int_t Constants::GetDefaultCanvasSize(const TString &coordinate){
    if(coordinate == "x" || coordinate == "X") return m_default_Canvas_XY[0];
    if(coordinate == "y" || coordinate == "Y") return m_default_Canvas_XY[1];
    return -1;
}


#endif
