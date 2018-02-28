//
//  DDLModule.cpp
//  AudioUnitV3Example
//
//  Created by MuE Student on 4/2/16.
//
//

#include "DDLModule.hpp"


CDDLModule::CDDLModule()
{

    m_fDelayInSamples=0;
    m_fFeedback=0;
    m_fWetLevel=0;
    m_nReadIndex=0;
    m_nWriteIndex=0;
    m_bUseExternalFeedback=false;
    m_bUseExternalXn=false;
    m_fExternalXn=0;
    m_fCurrentInput=0;
    
    resetDelay();
    cookVariables();

}
void CDDLModule::prepare()
{

    m_nBufferSize=2*m_nSampleRate;
    if(m_pBuffer)
        delete[] m_pBuffer;
    m_pBuffer=new float[m_nBufferSize];
    resetDelay();
    cookVariables();

}
void CDDLModule::cookVariables()
{
    m_nReadIndex=m_nWriteIndex-(int)m_fDelayInSamples;
    if(m_nReadIndex<0)
    {
    
        m_nReadIndex+=m_nBufferSize;
    }
    
}
void CDDLModule::resetDelay()
{

    if(m_pBuffer)
        memset(m_pBuffer,0.0,m_nBufferSize*sizeof(float));
    m_nWriteIndex=0;
    m_fCurrentInput=0;
    cookVariables();

}
float dLinTerp(float x1,float x2,float y1,float y2,float x)
{
    float denom=x2-x1;
    if(denom==0)
        return y1;
    float dx=(x-x1)/(x2-x1);
    float result=dx*y2+(1-dx)*y1;
    return result;
    
}
float CDDLModule::processAudio(float fInput)
{
    float yn=m_pBuffer[m_nReadIndex];
    m_fCurrentInput=fInput;
    if(m_fDelayInSamples==0)
    {
        yn=fInput;
    
    }
    if(m_nReadIndex==m_nWriteIndex&&m_fDelayInSamples<1.0)
        yn=fInput;
    int nReadIndex_1=m_nReadIndex-1;
    if(nReadIndex_1<0)
        nReadIndex_1=m_nBufferSize-1;
    float yn_1=m_pBuffer[nReadIndex_1];
    float fFracDelay=m_fDelayInSamples-(int)m_fDelayInSamples;
    float fInterp=dLinTerp(0, 1, yn, yn_1, fFracDelay);
    if(m_fDelayInSamples==0)
        yn=fInput;
    else
        yn=fInterp;
    if(m_bUseExternalXn)
        fInput=m_fExternalXn;
    
    if(!m_bUseExternalFeedback)
        m_pBuffer[m_nWriteIndex]=fInput+m_fFeedback*yn;
    else
        m_pBuffer[m_nWriteIndex]=fInput+m_fFeedBackIn;
    
    m_nWriteIndex++;
    if(m_nWriteIndex>=m_nBufferSize)
        m_nWriteIndex=0;
    m_nReadIndex++;
    if(m_nReadIndex>=m_nBufferSize)
        m_nReadIndex=0;
    float result=m_fWetLevel*yn+(1.0-m_fWetLevel)*fInput;
    return result;
    

}
