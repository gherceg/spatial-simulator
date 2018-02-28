//
//  SpatialAudioUnit.m
//  Capstone
//
//  Created by Graham Herceg on 3/10/17.
//  Copyright © 2017 GH. All rights reserved.
//

#import "SpatialAudioUnit.h"

#import <AVFoundation/AVFoundation.h>
#import "SpatialDSPKernel.hpp"
#import "BufferedAudioBus.hpp"

@interface SpatialAudioUnit ()
{
    // AUParameters
    AUParameter *azimuthLeftParam;
    AUParameter *azimuthRightParam;
    AUParameter *elevationLeftParam;
    AUParameter *elevationRightParam;
}

@property AUAudioUnitBus *outputBus;
@property AUAudioUnitBusArray *inputBusArray;
@property AUAudioUnitBusArray *outputBusArray;

@property (nonatomic, readwrite) AUParameterTree *parameterTree;

@end


@implementation SpatialAudioUnit {
    // C++ members need to be ivars; they would be copied on access if they were properties.
    // these provide objective-c wrappers to C++ objects
    SpatialDSPKernel _kernel;
    
    BufferedInputBus _inputBus;
}
@synthesize parameterTree = _parameterTree;

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription options:(AudioComponentInstantiationOptions)options error:(NSError **)outError {
    self = [super initWithComponentDescription:componentDescription options:options error:outError];
    
    if (self == nil) {
        return nil;
    }
    
    // Initialize a default format for the busses.
    AVAudioFormat *defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100. channels:2];
    
    // Create a DSP kernel to handle the signal processing.
    _kernel.init(defaultFormat.channelCount, defaultFormat.sampleRate);
    
    // Create an AU parameter object for Azimuth Left Angle
    azimuthLeftParam = [AUParameterTree createParameterWithIdentifier:@"azimuthLeft"
                                                                 name:@"AzimuthLeft"
                                                              address:ParamAzimuthLeft
                                                                  min:0.0
                                                                  max:359.0
                                                                 unit:kAudioUnitParameterUnit_Degrees
                                                             unitName:nil
                                                                flags:0
                                                         valueStrings:nil
                                                  dependentParameters:nil];
    // Initialize the parameter values
    azimuthLeftParam.value = 1.0;
    _kernel.setParameter(ParamAzimuthLeft, azimuthLeftParam.value);
    
    // Create an AU parameter object for Azimuth Right Angle
    azimuthRightParam = [AUParameterTree createParameterWithIdentifier:@"azimuthRight"
                                                                  name:@"AzimuthRight"
                                                               address:ParamAzimuthRight
                                                                   min:0.0
                                                                   max:359.0
                                                                  unit:kAudioUnitParameterUnit_Degrees
                                                              unitName:nil
                                                                 flags:0
                                                          valueStrings:nil
                                                   dependentParameters:nil];
    // Initialize the parameter values
    azimuthRightParam.value = 1.0;
    _kernel.setParameter(ParamAzimuthRight, azimuthRightParam.value);
    
    // Create an AU parameter object for Elevation Left Angle
    elevationLeftParam = [AUParameterTree createParameterWithIdentifier:@"elevationLeft"
                                                                   name:@"ElevationLeft"
                                                                address:ParamElevationLeft
                                                                    min:-90.0
                                                                    max:90.0
                                                                   unit:kAudioUnitParameterUnit_Degrees
                                                               unitName:nil
                                                                  flags:0
                                                           valueStrings:nil
                                                    dependentParameters:nil];
    // Initialize the parameter values
    elevationLeftParam.value = 1.0;
    _kernel.setParameter(ParamElevationLeft, elevationLeftParam.value);
    
    // Create an AU parameter object for Elevation Right Angle
    elevationRightParam = [AUParameterTree createParameterWithIdentifier:@"elevationRight"
                                                                    name:@"ElevationRight"
                                                                 address:ParamElevationRight
                                                                     min:-90.0
                                                                     max:90.0
                                                                    unit:kAudioUnitParameterUnit_Degrees
                                                                unitName:nil
                                                                   flags:0
                                                            valueStrings:nil
                                                     dependentParameters:nil];
    // Initialize the parameter values
    elevationRightParam.value = 1.0;
    _kernel.setParameter(ParamElevationRight, elevationRightParam.value);
    
    // Create the parameter tree.
    _parameterTree = [AUParameterTree createTreeWithChildren:@[
                                                               azimuthLeftParam,
                                                               azimuthRightParam,
                                                               elevationLeftParam,
                                                               elevationRightParam//, list more comma separated
                                                               ]];
    
    // Create the input and output busses.
    _inputBus.init(defaultFormat, 8);
    _outputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
    
    // Create the input and output bus arrays.
    _inputBusArray  = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeInput  busses: @[_inputBus.bus]];
    _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses: @[_outputBus]];
    
    // Make a local pointer to the kernel to avoid capturing self.
    __block SpatialDSPKernel *spatialKernel = &_kernel;
    
    // implementorValueObserver is called when a parameter changes value.
    _parameterTree.implementorValueObserver = ^(AUParameter *param, AUValue value) {
        spatialKernel->setParameter(param.address, value);
    };
    
    // implementorValueProvider is called when the value needs to be refreshed.
    _parameterTree.implementorValueProvider = ^(AUParameter *param) {
        return spatialKernel->getParameter(param.address);
    };
    
    // A function to provide string representations of parameter values.
    // should make as compact as possible
    _parameterTree.implementorStringFromValueCallback = ^(AUParameter *param, const AUValue *__nullable valuePtr) {
        AUValue value = valuePtr == nil ? param.value : *valuePtr;
        switch (param.address) {
            case ParamAzimuthLeft:
                return [NSString stringWithFormat:@"%.2f", value];
                
            case ParamAzimuthRight:
                return [NSString stringWithFormat:@"%.2f", value];
                
            case ParamElevationLeft:
                return [NSString stringWithFormat:@"%.2f", value];
                
            case ParamElevationRight:
                return [NSString stringWithFormat:@"%.2f", value];
            default:
                return @"?";
        }
    };
    
    self.maximumFramesToRender = 2048;
    
    return self;
}

#pragma mark - AUAudioUnit Overrides

- (AUAudioUnitBusArray *)inputBusses {
    return _inputBusArray;
}

- (AUAudioUnitBusArray *)outputBusses {
    return _outputBusArray;
}

- (BOOL)allocateRenderResourcesAndReturnError:(NSError **)outError {
    if (![super allocateRenderResourcesAndReturnError:outError]) {
        return NO;
    }
    
    if (self.outputBus.format.channelCount != _inputBus.bus.format.channelCount) {
        if (outError) {
            *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:kAudioUnitErr_FailedInitialization userInfo:nil];
        }
        // Notify superclass that initialization was not successful
        self.renderResourcesAllocated = NO;
        
        return NO;
    }
    
    _inputBus.allocateRenderResources(self.maximumFramesToRender);
    
    _kernel.init(self.outputBus.format.channelCount, self.outputBus.format.sampleRate);
    _kernel.reset();
    
    /*
     While rendering, we want to schedule all parameter changes. Setting them
     off the render thread is not thread safe.
     */
    __block AUScheduleParameterBlock scheduleParameter = self.scheduleParameterBlock;
    
    // Ramp over 20 milliseconds.
    __block AUAudioFrameCount rampTime = AUAudioFrameCount(0.02 * self.outputBus.format.sampleRate);
    
    self.parameterTree.implementorValueObserver = ^(AUParameter *param, AUValue value) {
        scheduleParameter(AUEventSampleTimeImmediate, rampTime, param.address, value);
    };
    
    return YES;
}

- (void)deallocateRenderResources {
    [super deallocateRenderResources];
    
    _inputBus.deallocateRenderResources();
    
    // Make a local pointer to the kernel to avoid capturing self.
    __block SpatialDSPKernel *spatialKernel = &_kernel;
    
    // Go back to setting parameters instead of scheduling them.
    self.parameterTree.implementorValueObserver = ^(AUParameter *param, AUValue value) {
        spatialKernel->setParameter(param.address, value);
    };
}

- (AUInternalRenderBlock)internalRenderBlock {
    /*
     Capture in locals to avoid ObjC member lookups. If "self" is captured in
     render, we're doing it wrong.
     */
    __block SpatialDSPKernel *state = &_kernel;
    __block BufferedInputBus *input = &_inputBus;
    
    return ^AUAudioUnitStatus(
                              AudioUnitRenderActionFlags *actionFlags,
                              const AudioTimeStamp       *timestamp,
                              AVAudioFrameCount           frameCount,
                              NSInteger                   outputBusNumber,
                              AudioBufferList            *outputData,
                              const AURenderEvent        *realtimeEventListHead,
                              AURenderPullInputBlock      pullInputBlock)
    
    //// start code completion block
    {
        AudioUnitRenderActionFlags pullFlags = 0;
        
        AUAudioUnitStatus err = input->pullInput(&pullFlags, timestamp, frameCount, 0, pullInputBlock);
        
        if (err != 0) {
            return err;
        }
        
        AudioBufferList *inAudioBufferList = input->mutableAudioBufferList;
        
        /*
         If the caller passed non-nil output pointers, use those. Otherwise,
         process in-place in the input buffer. If your algorithm cannot process
         in-place, then you will need to preallocate an output buffer and use
         it here.
         */
        AudioBufferList *outAudioBufferList = outputData;
        if (outAudioBufferList->mBuffers[0].mData == nullptr) {
            for (UInt32 i = 0; i < outAudioBufferList->mNumberBuffers; ++i) {
                outAudioBufferList->mBuffers[i].mData = inAudioBufferList->mBuffers[i].mData;
            }
        }
        
        state->setBuffers(inAudioBufferList, outAudioBufferList);
        // this calls a function that calls 'process()' inside SpatialDSPKernel.hpp
        state->processWithEvents(timestamp, frameCount, realtimeEventListHead);
        
        return noErr;
    };
    //// end code completion block
}

// Init Methods

// Methods to deal with UI Change

-(void)setAzimuthLeftAngle:(float)newAzimuthLeftAngle {
    _kernel.setParameter(ParamAzimuthLeft,newAzimuthLeftAngle);
}

-(void)setAzimuthRightAngle:(float)newAzimuthRightAngle {
    _kernel.setParameter(ParamAzimuthRight,newAzimuthRightAngle);
}

-(void)setElevationLeftAngle:(float)newElevationLeftAngle {
    _kernel.setParameter(ParamElevationLeft,newElevationLeftAngle);
}

-(void)setElevationRightAngle:(float)newElevationRightAngle {
    _kernel.setParameter(ParamElevationRight,newElevationRightAngle);
}

-(void)setDistanceLeft:(float)newDistance {
    _kernel.setParameter(ParamDistanceLeft,newDistance);
}

-(void)setDistanceRight:(float)newDistance {
    _kernel.setParameter(ParamDistanceRight,newDistance);
}

-(void)toggleHRTFMode:(bool)mode {
    _kernel.toggleHRTFMode(mode);
}

-(void)setGain:(float)newGain {
    _kernel.setGain(newGain);
}

@end

