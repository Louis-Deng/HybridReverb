//
//  ConvolutionManager.h
//  RTConvolve
//
//  Created by Graham Barab on 2/7/17.
//  Edited by Louis Deng in 2023, included changes to newer Juce version
//

#ifndef ConvolutionManager_h
#define ConvolutionManager_h

#include "UniformPartitionConvolver.h"
#include "TimeDistributedFFTConvolver.h"
#include <JuceHeader.h>
#include "util.h"
#include "SincFilter.hpp"
#include <memory>

static const int DEFAULT_NUM_SAMPLES = 512;
static const int DEFAULT_BUFFER_SIZE = 512;

template <typename FLOAT_TYPE>
class ConvolutionManager
{
public:
    ConvolutionManager(FLOAT_TYPE *impulseResponse = nullptr, int numSamples = 0, int bufferSize = 0)
    : mBufferSize(bufferSize)
    , mTimeDistributedConvolver(nullptr)
    {
        if (impulseResponse == nullptr)
        {
            mBufferSize = DEFAULT_BUFFER_SIZE;
            mImpulseResponse.reset( new juce::AudioBuffer<FLOAT_TYPE>(1, DEFAULT_NUM_SAMPLES) );
            checkNull(mImpulseResponse.get());
            
            FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
            genImpulse(ir, DEFAULT_NUM_SAMPLES);
            
            init(ir, DEFAULT_NUM_SAMPLES);
        }
        else
        {
            mImpulseResponse.reset( new juce::AudioBuffer<FLOAT_TYPE>(1, numSamples) );
            checkNull(mImpulseResponse.get());
            mImpulseResponse->clear();
            FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
            
            memcpy(ir, impulseResponse, numSamples * sizeof(FLOAT_TYPE));
            init(ir, numSamples);
        }
    }
    
    
    /**
     Perform one base time period's worth of work for the convolution.
     @param input
     The input is expected to hold a number of samples equal to the 'bufferSize'
     specified in the constructor.
     */
    void processInput(FLOAT_TYPE *input)
    {
        mUniformConvolver->processInput(input);
        const FLOAT_TYPE *out1 = mUniformConvolver->getOutputBuffer();
        FLOAT_TYPE *output = mOutput->getWritePointer(0);
        
        /* Prepare output */
        
        if (mTimeDistributedConvolver != nullptr)
        {
            mTimeDistributedConvolver->processInput(input);
            const FLOAT_TYPE *out2 = mTimeDistributedConvolver->getOutputBuffer();
            
            for (int i = 0; i < mBufferSize; ++i)
            {
                //output[i] = out1[i] + out2[i];
                //now adds dry/wet mix
                output[i] = dwmix*(out1[i]+out2[i])/1.0+(1.0-dwmix)*input[i];
            }
        }
        else
        {
            for (int i = 0; i < mBufferSize; ++i)
            {
                //output[i] = out1[i];
                //now adds dry/wet mix
                output[i] = dwmix*out1[i]/1.0+(1.0-dwmix)*input[i];
                
            }
        }
    }
    
    const FLOAT_TYPE *getOutputBuffer() const
    {
        return mOutput->getReadPointer(0);
    }
    
    void setBufferSize(int bufferSize)
    {
        FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
        int numSamples = mImpulseResponse->getNumSamples();
        mBufferSize = bufferSize;
        
        init(ir, numSamples);
    }
    
    void setImpulseResponse(const FLOAT_TYPE *impulseResponse, int numSamples)
    {
        mImpulseResponse.reset( new juce::AudioBuffer<FLOAT_TYPE>(1, numSamples) );
        checkNull(mImpulseResponse.get());
        
        mImpulseResponse->clear();
        
        FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
        memcpy(ir, impulseResponse, numSamples * sizeof(FLOAT_TYPE));
        init(ir, numSamples);
    }
    
    void changeMixe(FLOAT_TYPE mixe)
    {
        //mixe maps to dry/wet per tick
        if(mixe<0.0||mixe>1.0)
            
            DBG("conv mix out of bound!");
        else
            dwmix = mixe;
    }
    
private:
    int mBufferSize;
    FLOAT_TYPE dwmix;
    std::unique_ptr<UPConvolver<FLOAT_TYPE> > mUniformConvolver;
    std::unique_ptr<TimeDistributedFFTConvolver<FLOAT_TYPE> >  mTimeDistributedConvolver;
    std::unique_ptr<juce::AudioBuffer<FLOAT_TYPE> > mOutput;
    std::unique_ptr<juce::AudioBuffer<FLOAT_TYPE> > mImpulseResponse;
    
    void init(FLOAT_TYPE *impulseResponse, int numSamples)
    {
        mUniformConvolver.reset( new UPConvolver<FLOAT_TYPE>(impulseResponse, numSamples, mBufferSize, 8) );
        checkNull(mUniformConvolver.get());
        
        FLOAT_TYPE *subIR = impulseResponse + (8 * mBufferSize);
        int subNumSamples = numSamples - (8 * mBufferSize);
        
        if (subNumSamples > 0)
        {
            mTimeDistributedConvolver.reset( new TimeDistributedFFTConvolver<FLOAT_TYPE>(subIR, subNumSamples, mBufferSize) );
            checkNull(mTimeDistributedConvolver.get());
        }
        else
        {
            mTimeDistributedConvolver = nullptr;
        }
        
        mOutput.reset( new juce::AudioBuffer<FLOAT_TYPE>(1, mBufferSize) );
        checkNull(mOutput.get());
    }
};

#endif /* ConvolutionManager_h */
