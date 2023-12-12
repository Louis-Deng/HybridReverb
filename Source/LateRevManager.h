//
//  LateRevManager.h
//  LouisHybridVerb
//
//  Created by Louis Deng on 2023-05-29.
//

#ifndef LateRevManager_h
#define LateRevManager_h

/***************************************************/
/*! \class LateRevManager
    \brief late reverb manager class

    This class implements a instance-based FDN reverb

    by Louis Boyu Deng, 2022-2023
*/
/***************************************************/

#include "Stk.h"
#include "PoleZero.h"
#include "Delay.h"
#include "APF.h"
#include "LBCF.h"
#include "util.h"
#include <memory>
#include <cmath>

template <typename FLOAT_TYPE>
class LateRevManager
{
public:
    ///constructor
    LateRevManager(int bufferSize = 0, double inSampleRate = 48000.00)
    : mBufferSize(bufferSize),
    pred(128),
    gc(0.0),
    lp_d(0.0),
    gk(0.0),
    beta(0.0),
    ga(0.0)
    {
        stk::Stk::setSampleRate(inSampleRate);
        init();
        this->setParams();
    }
  
    /*
//    void changeMixe(FLOAT_TYPE mixe)
//    {
//        //mixe maps to dry/wet per tick
//        if(mixe<0.0||mixe>1.0)
//
//            DBG("laterev mix out of bound!");
//        else
//            dwmix = mixe;
//    }
//    void changeGain(FLOAT_TYPE gain)
//    {
//        //gain maps to gc
//        if(gain>=1.0)
//            DBG("laterev gain out of bound!");
//        else{
//            gc = gain;
//            for(int i=0;i<4;i++){
//                paraLBCF[i].loadParams(cf_l[i],gc,lp_d);
//            }
//        }
//
//
//    }
//
//    void changeDamp(FLOAT_TYPE damp)
//    {
//        //damp maps to lp_d
//        if(damp<0.0||damp>1.0)
//            DBG("laterev damp out of bound!");
//        else{
//            lp_d = damp;
//            for(int i=0;i<4;i++){
//                paraLBCF[i].loadParams(cf_l[i],gc,lp_d);
//            }
//        }
//
//    }
    */
    /// custom function for loading alternative params
    void loadParams(const unsigned long lbcf_in[4], const float gc_in, const float lp_d_in, const float gk_in, const float beta_in, const unsigned long ap_in[4], const float ga_in, const float allg_in, const unsigned long pred_in)
    {
        pred=pred_in;
        
        gc=gc_in;
        lp_d=lp_d_in;
        gk=gk_in;
        beta=beta_in;
        
        ga=ga_in;
        allg=allg_in;
        
        for(int i=0;i<4;i++){
            cf_l[i]=lbcf_in[i];
        }
        
        for(int i=0;i<4;i++){
            ap_l[i]=ap_in[i];
        }
        
        setParams();
    }
    
    void changeDecay(float changed_ga){
        ga = changed_ga;
        for (int i=0;i<4;i++){
            seriesAP[i].loadParams(ap_l[i],ga);
            //seriesAP[i].clear();
        }
    }
    
    void switchFdnMod(bool val){
        fdn_on = val;
    }
    
//==========================================================================================================================================================
    
    void processInput(const FLOAT_TYPE *input)
    {
        FLOAT_TYPE *output = mOutput->getWritePointer(0);
        
        for (int i=0;i<mBufferSize;i++){
            output[i] = this->tick(*(input+i));
        }
    }
    
    void setBufferSize(int bufferSize)
    {
        mBufferSize = bufferSize; 
        mOutput.reset( new juce::AudioBuffer<FLOAT_TYPE>(1, mBufferSize) );
        checkNull(mOutput.get());
    }
    
    void setSampleRate(double sampleRate)
    {
        stk::Stk::setSampleRate(sampleRate);
    }
    
    const FLOAT_TYPE *getOutputBuffer() const
    {
        return mOutput->getReadPointer(0);
    }
    
private:
    int mBufferSize;
    /// predelay length
    unsigned long pred;
    /// parameters for low pass feed back comb filter
    unsigned long cf_l[4]={1023,1023,1023,1023};
    FLOAT_TYPE gc;      //gain for LBCF
    FLOAT_TYPE lp_d;    //damping
    /// parameters for correction filter after lbcf
    FLOAT_TYPE gk;
    FLOAT_TYPE beta;
    /// parameters for ap filter
    FLOAT_TYPE ga;      //0.708
    unsigned long ap_l[4]={1023,1023,1023,1023};
    /// overall gain
    FLOAT_TYPE allg;
    /// dry/wet mix
    //FLOAT_TYPE dwmix;
    /// parameters for the matrix
    FLOAT_TYPE gmat = 1/sqrt(2);   // [Stewart, Murphy 2007], also jot
    std::vector<std::vector<FLOAT_TYPE>> matrixA = {{0.0f,gmat,gmat,0.0f},{-gmat,0.0f,0.0f,-gmat},{gmat,0.0f,0.0f,-gmat},{0.0f,gmat,-gmat,0.0f}};   // Jot 1991
    std::vector<std::vector<FLOAT_TYPE>> matrixTVM = {
        {0.99999999765531488726821862655925f,
        0.00003349508440321691580361315732f,
        -0.00005309965942764562746823686634f,
        -0.00002734731491344220718998567210f},
        {-0.00003349494694091056690458604872f,
        0.99999999756856638999380493260105f,
        -0.00002593860671744873671543812899f,
        0.00005539083408693785368385673062f},
        {0.00005309969732805521771233647765f,
        0.00002593854878617331868439349618f,
        0.99999999770416603261935506452573f,
        0.00003315541943693950946913870381f},
        {0.00002734740968684362436122610440f,
        -0.00005539077809123328665664998116f,
        -0.00003315543481435169622349723451f,
        0.99999999754234902837879417347722f}}; // generated using Schlecht 2020
    /// modules
    std::unique_ptr<stk::Delay> predL;
    stk::LBCF paraLBCF[4];
    std::unique_ptr<stk::PoleZero> corrF;
    stk::APF seriesAP[4];
    /// internal states
    std::vector<FLOAT_TYPE> splitinput = {0.0f,0.0f,0.0f,0.0f};
    std::vector<FLOAT_TYPE> lbcfout = {0.0f,0.0f,0.0f,0.0f};
    FLOAT_TYPE corrout;
    FLOAT_TYPE lastout;
    bool fdn_on = false;
    /// values
    std::unique_ptr<juce::AudioBuffer<FLOAT_TYPE> > mOutput;
    
    //*****************************************************************************
    //functions
    void init(){
        predL.reset(new stk::Delay(pred,8192));
        corrF.reset(new stk::PoleZero());
    }
    /**
     Perform one base time period's worth of work for late reverb.
     @param inL
     The input is expected to be a sample
     */
    FLOAT_TYPE tick(FLOAT_TYPE inL){
        
        //vin[1] = *inR;
        
        predL->tick(inL);

        for(int i=0;i<4;i++){
            splitinput[i]=predL->lastOut();
            lbcfout[i]=paraLBCF[i].lastOut();
        }

        //sum from matrix output before LBCF and do LBCF parallel
        vecmult(lbcfout, matrixA);

        for(int i=0;i<4;++i){
            splitinput[i] += lbcfout[i];
            lbcfout[i] = paraLBCF[i].tick(splitinput[i]);
        }

        //sum , go through correction filter, and go to allpass series
        corrout = corrF->tick(fourwaysum(lbcfout));

        for(int i=0;i<4;++i){
            corrout = seriesAP[i].tick(corrout);
        }
        
        if (fdn_on) matmult(matrixA, matrixTVM);
        
        lastout = inL+corrout*allg;
        return lastout;

    }
    
    /**
     Perform matrix multiplication of a 4x1 vector with 4x4 matrix
     @param invec[4]
     The invec is a 4x1 vector as input
     @param matri[4][4]
     The matri is a 4x4 matrix as input
     */
    void vecmult(std::vector<FLOAT_TYPE>& veci, std::vector<std::vector<FLOAT_TYPE>>& mati){
        std::vector<FLOAT_TYPE> vecout(4, 0.0f);
        for(int m=0;m<4;++m){
            for(int n=0;n<4;++n){
                vecout[m] += veci[n] * mati[m][n];
            }
            veci[m] = vecout[m];
        }
    }
    
    void matmult(std::vector<std::vector<FLOAT_TYPE>>& matA, std::vector<std::vector<FLOAT_TYPE>>& matrot){
        std::vector<std::vector<FLOAT_TYPE>> matout(4, std::vector<FLOAT_TYPE>(4, 0.0f));
        for (int m=0; m<4; m++) {
            for (int n=0; n<4; n++) {
                for (int i=0; i<4; i++) {
                    matout[m][n] += matA[m][i] * matrot[n][i];
                }
            }
        }
        matA = matout;
    }

    FLOAT_TYPE fourwaysum(std::vector<FLOAT_TYPE> invec){
        FLOAT_TYPE tmpsum = 0.0;
        for(int i=0;i<4;++i){
            tmpsum += invec[i];
        }
        return tmpsum;
    }
    
    void setParams(){
        predL->setDelay(pred);
        predL->clear();
        for (int i=0;i<4;i++){
            paraLBCF[i].loadParams(cf_l[i],gc,lp_d);
            paraLBCF[i].clear();
        }
        corrF->setCoefficients(gk/(1-beta),-gk*beta/(1-beta),0,false);
        corrF->clear();
        for (int i=0;i<4;i++){
            seriesAP[i].loadParams(ap_l[i],ga);
            seriesAP[i].clear();
        }
        for (int i=0;i<4;i++){
            splitinput[i]=0;
            lbcfout[i]=0;
        }
        corrout=0;
        lastout=0;
    }
    
    
};

#endif /* LateRevManager_h */
