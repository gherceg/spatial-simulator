//
//  SpatialAppViewController.h
//  Capstone
//
//  Created by Graham Herceg on 3/10/17.
//  Copyright © 2017 GH. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreAudioKit/CoreAudioKit.h>
#import "SpatialAudioUnit.h"

typedef NS_ENUM(NSInteger, PresetType) {
    PresetNearField = 0,
    PresetWideNearField,
    PresetReverseNearField,
    PresetFarField,
    PresetFrontBack,
    PresetAbove,
    PresetBelow,
    PresetNone
};


@interface SpatialAppViewController : AUViewController<AUAudioUnitFactory, UITableViewDelegate, UITableViewDataSource>

@property (nonatomic) AUParameterObserverToken parameterObserverToken;
@property (nonatomic, strong) SpatialAudioUnit* audioUnit;


@property (strong,nonatomic) NSArray* presetData;

@end
