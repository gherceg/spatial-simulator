//
//  SpatialDSPKernel.h
//  Capstone
//
//  Created by Graham Herceg on 3/10/17.
//  Copyright © 2017 GH. All rights reserved.
//

#ifndef SpatialDSPKernel_h
#define SpatialDSPKernel_h

#import "DSPKernel.hpp"
#import "ParameterRamper.hpp"
#import "FFTConvolver.hpp"
#import "IRArraySetter.hpp"
#import <vector>

#define NUM_OF_IRS 90
#define BUFFER_SIZE 1024
#define IR_SIZE 8192
#define ELEV_RAILS 4

static inline float convertBadValuesToZero(float x) {
    /*
     Eliminate denormals, not-a-numbers, and infinities.
     Denormals will fail the first test (absx > 1e-15), infinities will fail
     the second test (absx < 1e15), and NaNs will fail both tests. Zero will
     also fail both tests, but since it will get set to zero that is OK.
     */
    
    float absx = fabs(x);
    
    if (absx > 1e-15 && absx < 1e15) {
        return x;
    }
    
    return 0.0;
}


enum {
    ParamAzimuthLeft,
    ParamAzimuthRight,
    ParamElevationLeft,
    ParamElevationRight,
    ParamDistanceLeft,
    ParamDistanceRight
};

static inline double squared(double x) {
    return x * x;
}

/*
	SpatialDSPKernel
	Performs our filter signal processing.
	As a non-ObjC class, this is safe to use from render thread.
 */
class SpatialDSPKernel : public DSPKernel {
public:
    
    SpatialDSPKernel() {}
    
    void init(int channelCount, double inSampleRate) {
        numChans = channelCount;
        sampleRate = float(inSampleRate);
        nyquist = 0.5 * sampleRate;
        // Set Convolution Length
        m_nConvolutionLength = 8192;

        // Set IRs for Elevation = 0°
        m_IRArraySetter.setIRsForE0(m_pIRs_E0_L,m_pIRs_E0_R);
        m_IRArraySetter.setIRsForE45(m_pIRs_E45_L,m_pIRs_E45_R);
        m_IRArraySetter.setIRsForE75(m_pIRs_E75_L,m_pIRs_E75_R);
        m_IRArraySetter.setIRsForE315(m_pIRs_E315_L,m_pIRs_E315_R);

        m_ppIRs_L_AziRails[0] = m_pIRs_E315_L;
        m_ppIRs_L_AziRails[1] = m_pIRs_E0_L;
        m_ppIRs_L_AziRails[2] = m_pIRs_E45_L;
        m_ppIRs_L_AziRails[3] = m_pIRs_E75_L;
        
        m_ppIRs_R_AziRails[0] = m_pIRs_E315_R;
        m_ppIRs_R_AziRails[1] = m_pIRs_E0_R;
        m_ppIRs_R_AziRails[2] = m_pIRs_E45_R;
        m_ppIRs_R_AziRails[3] = m_pIRs_E75_R;
        
        // Set fftConvolvers Left and Right
        fftConvolver_srcL_L.init(m_nConvolutionLength,m_pIRs_E0_L[60],m_nConvolutionLength);
        fftConvolver_srcL_R.init(m_nConvolutionLength,m_pIRs_E0_R[60],m_nConvolutionLength);
        fftConvolver_srcR_L.init(m_nConvolutionLength,m_pIRs_E0_L[60],m_nConvolutionLength);
        fftConvolver_srcR_R.init(m_nConvolutionLength,m_pIRs_E0_R[60],m_nConvolutionLength);
        
        fftConvolverPrev_srcL_L.init(m_nConvolutionLength,m_pIRs_E0_L[60],m_nConvolutionLength);
        fftConvolverPrev_srcL_R.init(m_nConvolutionLength,m_pIRs_E0_R[60],m_nConvolutionLength);
        fftConvolverPrev_srcR_L.init(m_nConvolutionLength,m_pIRs_E0_L[60],m_nConvolutionLength);
        fftConvolverPrev_srcR_R.init(m_nConvolutionLength,m_pIRs_E0_R[60],m_nConvolutionLength);

        m_bPosChanged_srcL = false;
        m_bPosChanged_srcR = false;
        
        m_fGain = 1.0;
    
        m_pCurrentIR_srcL_L = NULL;
        m_pCurrentIR_srcL_R = NULL;
        m_pCurrentIR_srcR_L = NULL;
        m_pCurrentIR_srcR_L = NULL;
        
        m_pPreviousIR_srcL_L = NULL;
        m_pPreviousIR_srcL_R = NULL;
        m_pPreviousIR_srcR_L = NULL;
        m_pPreviousIR_srcR_R = NULL;
        
        m_pCurrentOutput_srcL_L = NULL;
        m_pCurrentOutput_srcL_R = NULL;
        m_pCurrentOutput_srcR_L = NULL;
        m_pCurrentOutput_srcR_R = NULL;
        
        m_pPreviousOutput_srcL_L = NULL;
        m_pPreviousOutput_srcL_R = NULL;
        m_pPreviousOutput_srcR_L = NULL;
        m_pPreviousOutput_srcR_R = NULL;
        
        
        //Clearing IRs
        if (m_pCurrentIR_srcL_L)
            delete [] m_pCurrentIR_srcL_L;
        if (m_pCurrentIR_srcL_R)
            delete [] m_pCurrentIR_srcL_R;
        if (m_pCurrentIR_srcR_L)
            delete [] m_pCurrentIR_srcR_L;
        if (m_pCurrentIR_srcR_R)
            delete [] m_pCurrentIR_srcR_R;
        
        if (m_pPreviousIR_srcL_L)
            delete [] m_pPreviousIR_srcL_L;
        if (m_pPreviousIR_srcL_R)
            delete [] m_pPreviousIR_srcL_R;
        if (m_pPreviousIR_srcR_L)
            delete [] m_pPreviousIR_srcR_L;
        if (m_pPreviousIR_srcR_R)
            delete [] m_pPreviousIR_srcR_R;
        
        if (m_pCurrentOutput_srcL_L)
            delete [] m_pCurrentOutput_srcL_L;
        if (m_pCurrentOutput_srcL_R)
            delete [] m_pCurrentOutput_srcL_R;
        if (m_pCurrentOutput_srcR_L)
            delete [] m_pCurrentOutput_srcR_L;
        if (m_pCurrentOutput_srcR_R)
            delete [] m_pCurrentOutput_srcR_R;
        
        if (m_pPreviousOutput_srcL_L)
            delete [] m_pPreviousOutput_srcL_L;
        if (m_pPreviousOutput_srcL_R)
            delete [] m_pPreviousOutput_srcL_R;
        if (m_pPreviousOutput_srcR_L)
            delete [] m_pPreviousOutput_srcR_L;
        if (m_pPreviousOutput_srcR_R)
            delete [] m_pPreviousOutput_srcR_R;
        
        //Allocating memory based on convolution length
        m_pCurrentIR_srcL_L = new float[IR_SIZE];
        m_pCurrentIR_srcL_R = new float[IR_SIZE];
        m_pCurrentIR_srcR_L = new float[IR_SIZE];
        m_pCurrentIR_srcR_R = new float[IR_SIZE];
        
        m_pPreviousIR_srcL_L = new float[IR_SIZE];
        m_pPreviousIR_srcL_R = new float[IR_SIZE];
        m_pPreviousIR_srcR_L = new float[IR_SIZE];
        m_pPreviousIR_srcR_R = new float[IR_SIZE];
        
        m_pCurrentOutput_srcL_L = new float[BUFFER_SIZE];
        m_pCurrentOutput_srcL_R = new float[BUFFER_SIZE];
        m_pCurrentOutput_srcR_L = new float[BUFFER_SIZE];
        m_pCurrentOutput_srcR_R = new float[BUFFER_SIZE];
        
        m_pPreviousOutput_srcL_L = new float[BUFFER_SIZE];
        m_pPreviousOutput_srcL_R = new float[BUFFER_SIZE];
        m_pPreviousOutput_srcR_L = new float[BUFFER_SIZE];
        m_pPreviousOutput_srcR_R = new float[BUFFER_SIZE];

        //Setting to 0
        memset(m_pCurrentIR_srcL_L, 0, sizeof(float)*IR_SIZE);
        memset(m_pCurrentIR_srcL_R, 0, sizeof(float)*IR_SIZE);
        memset(m_pCurrentIR_srcR_L, 0, sizeof(float)*IR_SIZE);
        memset(m_pCurrentIR_srcR_R, 0, sizeof(float)*IR_SIZE);
        
        memset(m_pPreviousIR_srcL_L, 0, sizeof(float)*IR_SIZE);
        memset(m_pPreviousIR_srcL_R, 0, sizeof(float)*IR_SIZE);
        memset(m_pPreviousIR_srcR_L, 0, sizeof(float)*IR_SIZE);
        memset(m_pPreviousIR_srcR_R, 0, sizeof(float)*IR_SIZE);
        
        memset(m_pCurrentOutput_srcL_L, 0, sizeof(float)*BUFFER_SIZE);
        memset(m_pCurrentOutput_srcL_R, 0, sizeof(float)*BUFFER_SIZE);
        memset(m_pCurrentOutput_srcR_L, 0, sizeof(float)*BUFFER_SIZE);
        memset(m_pCurrentOutput_srcR_R, 0, sizeof(float)*BUFFER_SIZE);
        
        memset(m_pPreviousOutput_srcL_L, 0, sizeof(float)*BUFFER_SIZE);
        memset(m_pPreviousOutput_srcL_R, 0, sizeof(float)*BUFFER_SIZE);
        memset(m_pPreviousOutput_srcR_L, 0, sizeof(float)*BUFFER_SIZE);
        memset(m_pPreviousOutput_srcR_R, 0, sizeof(float)*BUFFER_SIZE);
        
        // Set Variables
        // GUI-> Variables
//        m_bHRTFMode = false;
        m_bPosChanged_srcL = true;
        m_bPosChanged_srcR = true;
        
        m_nIndex_PrevAzi_srcL = 1;
        m_nIndex_PrevAzi_srcR = 1;
        m_nIndex_PrevElev_srcL = 1;
        m_nIndex_PrevElev_srcR = 1;
        
        m_fPreviousRemainder = 0;
        
        m_fDistance_srcL = 1.0;
        m_fDistance_srcR = 1.0;
        
        m_bSwitching = true;
        m_bTwoSources = true;
        
        // call any necessary functions
        createFaders();
    }
    
    void reset() {
        // reset and state variables here (eg, filter delays)
    }
    
    void createFaders() {
        for(int i = 0; i < BUFFER_SIZE; i++) {
            m_pFadingVector_1[i] = i/(float)(BUFFER_SIZE-1);
            m_pFadingVector_2[i] = 1 - (i/(float)(BUFFER_SIZE-1));
        }
    }
    
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case ParamAzimuthLeft:
                azimuthLeftRamper.set(clamp(value, 2.0f, 10.0f));
                m_fCurrentAzimuth_srcL = value;
                m_bPosChanged_srcL = true;
                break;
            case ParamAzimuthRight:
                azimuthRightRamper.set(clamp(value, 2.0f, 10.0f));
                m_fCurrentAzimuth_srcR = value;
                m_bPosChanged_srcR = true;
                break;
            case ParamElevationLeft:
                elevationLeftRamper.set(clamp(value,100.0f, 10000.0f));
                m_fCurrentElevation_srcL = value;
                m_bPosChanged_srcL = true;
                break;
            case ParamElevationRight:
                elevationRightRamper.set(clamp(value,100.0f, 10000.0f));
                m_fCurrentElevation_srcR = value;
                m_bPosChanged_srcR = true;
                break;
            case ParamDistanceLeft:
                distanceLeftRamper.set(clamp(value,100.0f, 10000.0f));
                m_fDistance_srcL = value;
                break;
            case ParamDistanceRight:
                distanceRightRamper.set(clamp(value,100.0f, 10000.0f));
                m_fDistance_srcR = value;
                break;
                
        }
    }
    
    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case ParamAzimuthLeft:
                return azimuthLeftRamper.goal();
                
            case ParamAzimuthRight:
                return azimuthRightRamper.goal();
                
            case ParamElevationLeft:
                return elevationLeftRamper.goal();
                
            case ParamElevationRight:
                return elevationRightRamper.goal();
                
            case ParamDistanceLeft:
                return distanceLeftRamper.goal();
                
            case ParamDistanceRight:
                return distanceRightRamper.goal();
                
            default: return 0.0f;
        }
    }
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration) override {
        switch (address) {
                // Change the clamp values
            case ParamAzimuthLeft:
                azimuthLeftRamper.startRamp(clamp(value,2.0f,10.0f), duration);
                
            case ParamAzimuthRight:
                azimuthRightRamper.startRamp(clamp(value,2.0f,10.0f), duration);
                
            case ParamElevationLeft:
                elevationLeftRamper.startRamp(clamp(value,2.0f,10.0f), duration);
                
            case ParamElevationRight:
                elevationRightRamper.startRamp(clamp(value,2.0f,10.0f), duration);
                
            case ParamDistanceLeft:
                distanceLeftRamper.startRamp(clamp(value,2.0f,10.0f),duration);
                
            case ParamDistanceRight:
                distanceRightRamper.startRamp(clamp(value,2.0f,10.0f),duration);
        }
    }
    
    void setBuffers(AudioBufferList* inBufferList, AudioBufferList* outBufferList) {
        inBufferListPtr = inBufferList;
        outBufferListPtr = outBufferList;
    }
    
    void quantize2D(int elevIndex, int aziIndex, bool source) {
        // bool source is 0 for Left, 1 for Right
        
        // Left Source
        if(!source) {
            fftConvolver_srcL_L.reset();
            fftConvolver_srcL_R.reset();
            fftConvolverPrev_srcL_L.reset();
            fftConvolverPrev_srcL_R.reset();
            
            // Find Correct Azimuth Rail (based on elevation)
            float** pAziRail_srcL_L = m_ppIRs_L_AziRails[elevIndex];
            float** pAziRail_srcL_R = m_ppIRs_R_AziRails[elevIndex];
            float** pPreviousAziRail_srcL_L = m_ppIRs_L_AziRails[m_nIndex_PrevElev_srcL];
            float** pPreviousAziRail_srcL_R = m_ppIRs_R_AziRails[m_nIndex_PrevElev_srcL];

            // Left ear
            fftConvolver_srcL_L.init(m_nConvolutionLength,pAziRail_srcL_L[aziIndex],m_nConvolutionLength);
            fftConvolverPrev_srcL_L.init(m_nConvolutionLength,pPreviousAziRail_srcL_L[m_nIndex_PrevAzi_srcL],m_nConvolutionLength);
            // Right ear
            fftConvolver_srcL_R.init(m_nConvolutionLength,pAziRail_srcL_R[aziIndex],m_nConvolutionLength);
            fftConvolverPrev_srcL_R.init(m_nConvolutionLength,pPreviousAziRail_srcL_R[m_nIndex_PrevAzi_srcL],m_nConvolutionLength);
            // Set current IR index to previous
            m_nIndex_PrevElev_srcL = elevIndex;
            m_nIndex_PrevAzi_srcL = aziIndex;
            
        }
        
        // Right Source
        else {
            fftConvolver_srcR_L.reset();
            fftConvolver_srcR_R.reset();
            fftConvolverPrev_srcR_L.reset();
            fftConvolverPrev_srcR_R.reset();
            
            // Find Correct Azimuth Rail (based on elevation)
            float** pAziRail_srcR_L = m_ppIRs_L_AziRails[elevIndex];
            float** pAziRail_srcR_R = m_ppIRs_R_AziRails[elevIndex];
            float** pPreviousAziRail_srcR_L = m_ppIRs_L_AziRails[m_nIndex_PrevElev_srcL];
            float** pPreviousAziRail_srcR_R = m_ppIRs_R_AziRails[m_nIndex_PrevElev_srcL];
            
            // Left ear
            fftConvolver_srcR_L.init(m_nConvolutionLength,pAziRail_srcR_L[aziIndex],m_nConvolutionLength);
            fftConvolverPrev_srcR_L.init(m_nConvolutionLength,pPreviousAziRail_srcR_L[m_nIndex_PrevAzi_srcR],m_nConvolutionLength);
            // Right ear
            fftConvolver_srcR_R.init(m_nConvolutionLength,pAziRail_srcR_R[aziIndex],m_nConvolutionLength);
            fftConvolverPrev_srcR_R.init(m_nConvolutionLength,pPreviousAziRail_srcR_R[m_nIndex_PrevAzi_srcR],m_nConvolutionLength);
            // Set current IR index to previous
            m_nIndex_PrevElev_srcR = elevIndex;
            m_nIndex_PrevAzi_srcR = aziIndex;
        }
    }
    
    void sumWithSwitching(float* leftOutput,float* rightOutput,bool source) {
        // should be switching between previous IR output (sum of sources at left ear and same for right ear)
        // and the current IR output
        // Left Source
        if(!source) {
            for(int i = 0; i < BUFFER_SIZE; i++) {
                // everything that goes to the left ear
                leftOutput[i] = (m_pFadingVector_2[i]*m_pPreviousOutput_srcL_L[i]) + (m_pFadingVector_1[i]*m_pCurrentOutput_srcL_L[i]);
                // everything that goes to the right ear
                rightOutput[i] = (m_pFadingVector_2[i]*m_pPreviousOutput_srcL_R[i]) + (m_pFadingVector_1[i]*m_pCurrentOutput_srcL_R[i]);
            }
        }
        // Right Source
        else {
            for(int i = 0; i < BUFFER_SIZE; i++) {
    
                leftOutput[i] = (m_pFadingVector_2[i]*m_pPreviousOutput_srcR_L[i]) + (m_pFadingVector_1[i]*m_pCurrentOutput_srcR_L[i]);
                rightOutput[i] = (m_pFadingVector_2[i]*m_pPreviousOutput_srcR_R[i]) + (m_pFadingVector_1[i]*m_pCurrentOutput_srcR_R[i]);
            }
        }
    }
    
    void sumOutput(float* leftOutput, float* rightOutput) {
        
        float gain_left = 1.0 / (m_fDistance_srcL);
        float gain_right = 1.0 / (m_fDistance_srcR);
        
        if(m_bTwoSources) {
            for(int i = 0; i < BUFFER_SIZE; i++) {
                // everything that goes to the left ear
                leftOutput[i] = gain_left*m_pCurrentOutput_srcL_L[i] + gain_right*m_pCurrentOutput_srcR_L[i];
                // everything that goes to the right ear
                rightOutput[i] = gain_left*m_pCurrentOutput_srcL_R[i] + gain_right*m_pCurrentOutput_srcR_R[i];
            }
        }
        
        // Only using left source (srcL)
        else {
            for(int i = 0; i < BUFFER_SIZE; i++) {
                leftOutput[i] = gain_left*m_pCurrentOutput_srcL_L[i];
                rightOutput[i] = gain_right*m_pCurrentOutput_srcL_R[i];
            }
        }
    }

    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset) override {
        
        // initialize any variables
        int index_IR_srcL=0,index_IR_srcR=0;
        float indexWithDec_srcL,indexWithDec_srcR;
        float remainder_srcL=0,remainder_srcR=0;
        
        // For 2D
        int aziIndex_srcL=0,aziIndex_srcR=0;
        int elevIndex_srcL=0,elevIndex_srcR=0;
        
        if(m_bHRTFMode) {
        //         Check if position changed for either or both sources
            if(m_bPosChanged_srcL) {
            
                indexWithDec_srcL = m_fCurrentAzimuth_srcL*(NUM_OF_IRS-1);
                index_IR_srcL = floor(indexWithDec_srcL);
                aziIndex_srcL = index_IR_srcL;
                elevIndex_srcL = findClosetElevation(m_fCurrentElevation_srcL);
                remainder_srcL = indexWithDec_srcL - index_IR_srcL;
                
                // Quantize to nearest IR
                if(elevIndex_srcL != m_nIndex_PrevElev_srcL || aziIndex_srcL != m_nIndex_PrevAzi_srcL)
                    quantize2D(elevIndex_srcL,aziIndex_srcL,false);

            }
            if(m_bPosChanged_srcR) {
                
                indexWithDec_srcR = m_fCurrentAzimuth_srcR*(NUM_OF_IRS-1);
                index_IR_srcR = floor(indexWithDec_srcR);
                aziIndex_srcR = index_IR_srcR;
                elevIndex_srcR = findClosetElevation(m_fCurrentElevation_srcR);
                remainder_srcR = indexWithDec_srcR - index_IR_srcR;
                
                // Quantize to nearest IR
                if(elevIndex_srcR != m_nIndex_PrevElev_srcR || aziIndex_srcR != m_nIndex_PrevAzi_srcR)
                    quantize2D(elevIndex_srcR,aziIndex_srcR,true);

            }
    
            // DO LEFT CHANNEL
            // Set pointers to input/output LEFT buffer
            float* xSrcL = (float*)inBufferListPtr->mBuffers[0].mData;
            float* ySrcL = (float*)outBufferListPtr->mBuffers[0].mData;
            // Set pointers to input/output RIGHT buffer
            float* xSrcR = (float*)inBufferListPtr->mBuffers[1].mData;
            float* ySrcR = (float*)outBufferListPtr->mBuffers[1].mData;
            
            fftConvolver_srcL_L.process(xSrcL,m_pCurrentOutput_srcL_L,BUFFER_SIZE);
            fftConvolver_srcL_R.process(xSrcL,m_pCurrentOutput_srcL_R,BUFFER_SIZE);
            
            if(m_bPosChanged_srcL) {
                // Need to process previous IR
                fftConvolverPrev_srcL_L.process(xSrcL,m_pPreviousOutput_srcL_L,BUFFER_SIZE);
                fftConvolverPrev_srcL_R.process(xSrcL,m_pPreviousOutput_srcL_R,BUFFER_SIZE);
                m_bPosChanged_srcL = false;
                
                sumWithSwitching(m_pCurrentOutput_srcL_L,m_pCurrentOutput_srcL_R,false);
            }

            if(m_bTwoSources) {
                // DO RIGHT CHANNEL
                fftConvolver_srcR_L.process(xSrcR,m_pCurrentOutput_srcR_L,BUFFER_SIZE);
                fftConvolver_srcR_R.process(xSrcR,m_pCurrentOutput_srcR_R,BUFFER_SIZE);
                
                if(m_bPosChanged_srcR) {
                    // Need to process previous IR
                    fftConvolverPrev_srcR_L.process(xSrcR,m_pPreviousOutput_srcR_L,BUFFER_SIZE);
                    fftConvolverPrev_srcR_R.process(xSrcR,m_pPreviousOutput_srcR_R,BUFFER_SIZE);
                    m_bPosChanged_srcR = false;
                    
                    sumWithSwitching(m_pCurrentOutput_srcR_L,m_pCurrentOutput_srcR_R,true);
                }
            }
            
            sumOutput(ySrcL,ySrcR);
        }
        
        // ELSE just pass audio through, unprocessed
        else {
            outBufferListPtr->mBuffers[0] = inBufferListPtr->mBuffers[0];
            outBufferListPtr->mBuffers[1] = inBufferListPtr->mBuffers[1];
        }
        if(m_bHRTFMode) {
            // Gain (in addition to distance) should tune this
            for(int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
                int frameOffset = int(frameIndex + bufferOffset);
                float* yLeft = (float*)outBufferListPtr->mBuffers[0].mData + frameOffset;
                float* yRight = (float*)outBufferListPtr->mBuffers[1].mData + frameOffset;
                *yLeft = *yLeft * 2.0;
                *yRight = *yRight * 2.0;
            }
        }
    }
    
    // Get/Set Methods
    void toggleHRTFMode(bool mode) {
        m_bHRTFMode = mode;
    }
    
    void setGain(float gainValue) {
        m_fGain = gainValue;
    }
    
    
    int findClosetElevation(float elevation) {
        float indexWithDec = elevation * (ELEV_RAILS - 1);

        return round(indexWithDec);
    }

    
    
    
    
    // MARK: Member Variables
private:
    
    float sampleRate = 44100.0;
    float nyquist = 0.5 * sampleRate;
    float numChans = 1;
    
    AudioBufferList* inBufferListPtr = nullptr;
    AudioBufferList* outBufferListPtr = nullptr;
    
    // Quantization Method
    // requires 2 IRs (L+R) for each source
    // if using one source, use left
    // m_pCurrentIR_<source>_<ear>
    float* m_pCurrentIR_srcL_L;
    float* m_pCurrentIR_srcL_R;
    // if using two sources, right source
    float* m_pCurrentIR_srcR_L;
    float* m_pCurrentIR_srcR_R;
    
    // Previous IRs for implementing switching technique
    float* m_pPreviousIR_srcL_L;
    float* m_pPreviousIR_srcL_R;
    // right source
    float* m_pPreviousIR_srcR_L;
    float* m_pPreviousIR_srcR_R;
    
    // Output pointers
    float* m_pCurrentOutput_srcL_L;
    float* m_pCurrentOutput_srcL_R;
    float* m_pCurrentOutput_srcR_L;
    float* m_pCurrentOutput_srcR_R;
    
    float* m_pPreviousOutput_srcL_L;
    float* m_pPreviousOutput_srcL_R;
    float* m_pPreviousOutput_srcR_L;
    float* m_pPreviousOutput_srcR_R;
    
    float m_pFadingVector_1[BUFFER_SIZE];
    float m_pFadingVector_2[BUFFER_SIZE];
    // ------------------------
    
    // Sets up HRIRs to arrays
    IRArraySetter m_IRArraySetter;
    
    // Pointer Array to IRS
    float* m_pIRs_E0_L[NUM_OF_IRS];
    float* m_pIRs_E0_R[NUM_OF_IRS];
//    float* m_pIRs_E15_L[NUM_OF_IRS];
//    float* m_pIRs_E15_R[NUM_OF_IRS];
//    float* m_pIRs_E30_L[NUM_OF_IRS];
//    float* m_pIRs_E30_R[NUM_OF_IRS];
    float* m_pIRs_E45_L[NUM_OF_IRS];
    float* m_pIRs_E45_R[NUM_OF_IRS];
//    float* m_pIRs_E60_L[NUM_OF_IRS];
//    float* m_pIRs_E60_R[NUM_OF_IRS];
    float* m_pIRs_E75_L[NUM_OF_IRS];
    float* m_pIRs_E75_R[NUM_OF_IRS];
    float* m_pIRs_E315_L[NUM_OF_IRS];
    float* m_pIRs_E315_R[NUM_OF_IRS];
//    float* m_pIRs_E330_L[NUM_OF_IRS];
//    float* m_pIRs_E330_R[NUM_OF_IRS];
//    float* m_pIRs_E345_L[NUM_OF_IRS];
//    float* m_pIRs_E345_R[NUM_OF_IRS];
    
    float** m_ppIRs_L_AziRails[ELEV_RAILS];
    float** m_ppIRs_R_AziRails[ELEV_RAILS];
    
    // float values for current azimuth angles
    float m_fCurrentAzimuth_srcL;
    float m_fCurrentAzimuth_srcR;
    float m_fCurrentElevation_srcL;
    float m_fCurrentElevation_srcR;
    float m_fDistance_srcL;
    float m_fDistance_srcR;
    
    // bools for position changing
    bool m_bPosChanged_srcL;
    bool m_bPosChanged_srcR;
    bool m_bSwitching;
    bool m_bTwoSources;
    bool m_bQuantizedIRs;
    
    // Need index to access array for previous IR
    int m_nIndex_PrevAzi_srcL;
    int m_nIndex_PrevAzi_srcR;
    int m_nIndex_PrevElev_srcL;
    int m_nIndex_PrevElev_srcR;

    float m_fPreviousRemainder;

    // gain
    float m_fGain;
    
    // convolution length (8192)
    int m_nConvolutionLength;
    
    
public:
    
    // Parameters.
    ParameterRamper azimuthLeftRamper = 0.0;
    ParameterRamper azimuthRightRamper = 0.0;
    ParameterRamper elevationLeftRamper = 0.0;
    ParameterRamper elevationRightRamper = 0.0;
    ParameterRamper distanceLeftRamper = 0.0;
    ParameterRamper distanceRightRamper = 0.0;

    
    // Public variables
    bool m_bHRTFMode;
    
    fftconvolver::FFTConvolver fftConvolver_srcL_L;
    fftconvolver::FFTConvolver fftConvolver_srcL_R;
    fftconvolver::FFTConvolver fftConvolver_srcR_L;
    fftconvolver::FFTConvolver fftConvolver_srcR_R;
    
    fftconvolver::FFTConvolver fftConvolverPrev_srcL_L;
    fftconvolver::FFTConvolver fftConvolverPrev_srcL_R;
    fftconvolver::FFTConvolver fftConvolverPrev_srcR_L;
    fftconvolver::FFTConvolver fftConvolverPrev_srcR_R;

};


#endif /* SpatialDSPKernel_hpp */
