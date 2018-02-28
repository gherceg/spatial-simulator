//
//  SpatialAudioUnit.h
//  Capstone
//
//  Created by Graham Herceg on 3/10/17.
//  Copyright © 2017 GH. All rights reserved.
//

#import <AudioToolbox/AudioToolbox.h>

@interface SpatialAudioUnit : AUAudioUnit

-(void)setAzimuthLeftAngle:(float)newAzimuthLeftAngle;
-(void)setAzimuthRightAngle:(float)newAzimuthRightAngle;
-(void)setElevationLeftAngle:(float)newElevationLeftAngle;
-(void)setElevationRightAngle:(float)newElevationRightAngle;
-(void)setDistanceLeft:(float)newDistance;
-(void)setDistanceRight:(float)newDistance;
-(void)setGain:(float)newGain;

-(void)toggleHRTFMode:(bool)mode;

@end
