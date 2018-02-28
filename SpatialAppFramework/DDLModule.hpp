//
//  DDLModule.hpp
//  AudioUnitV3Example
//
//  Created by MuE Student on 4/2/16.
//
//

#ifndef DDLModule_hpp
#define DDLModule_hpp

#include <stdio.h>
#include<stdlib.h>
#include <cstring>



class CDDLModule
{
public:
    CDDLModule();
    void cookVariables();
    void resetDelay();
    void prepare();
    float m_fFeedback;
    float m_fWetLevel;
    float m_fDelayInSamples;
    float processAudio(float fInput);
    int m_nSampleRate;
    float m_fExternalXn;
    bool m_bUseExternalFeedback;
    bool m_bUseExternalXn;
    float m_fFeedBackIn;
    float m_fCurrentInput;
    
    float getCurrentFeedBackOutput(){return m_fFeedback*m_pBuffer[m_nReadIndex];}
    float getDelayOutput(){return m_pBuffer[m_nReadIndex];}
    float getCurrentInput(){return m_fCurrentInput;}
    
private:
    
    float* m_pBuffer;
    int m_nReadIndex,m_nWriteIndex,m_nBufferSize;
    
    


};
#endif /* DDLModule_hpp */