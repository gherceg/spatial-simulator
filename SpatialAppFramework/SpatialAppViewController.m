//
//  SpatialAppViewController.m
//  Capstone
//
//  Created by Graham Herceg on 3/10/17.
//  Copyright © 2017 GH. All rights reserved.
//

#import "SpatialAppViewController.h"
#import "EFCircularSlider.h"
#import "VerticalSlider.h"
//#import "Constants.h"



@interface SpatialAppViewController ()
{
    // private properties
    bool HRTFMode;
    
    // IBOutlets here
    // Outlets for th Left Speaker

    __weak IBOutlet VerticalSlider *elevationLeftSlider;
    __weak IBOutlet UISlider *distanceLeftSlider;
    __weak IBOutlet UITextField *elevationLeftText;
    __weak IBOutlet UITextField *distanceLeftText;
    
    // Outlets for the Right Speaker

    __weak IBOutlet VerticalSlider *elevationRightSlider;
    __weak IBOutlet UISlider *distanceRightSlider;
    
    __weak IBOutlet UITextField *elevationRightText;
    __weak IBOutlet UITextField *distanceRightText;
    
    __weak IBOutlet EFCircularSlider *azimuthLeftCircularSlider;
    __weak IBOutlet UILabel *aziLeftLabel;
    
    __weak IBOutlet EFCircularSlider *azimuthRightCircularSlider;
    __weak IBOutlet UILabel *aziRightLabel;
    
    // Turn convolution on/off
    __weak IBOutlet UISwitch *hrtfModeSwitch;
    
    // Preset List
    __weak IBOutlet UIButton *presetButton;
    __weak IBOutlet UITableView *presetTableView;
    
}
// IBAction Declarations
- (IBAction)elevationLeftSliderChanged:(id)sender;
- (IBAction)distanceLeftSliderChanged:(id)sender;
- (IBAction)elevationRightSliderChanged:(id)sender;
- (IBAction)distanceRightSliderChanged:(id)sender;
- (IBAction)hrtfModeSwitched:(id)sender;
- (IBAction)presetButtonPressed:(id)sender;




@end

@implementation SpatialAppViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    self->HRTFMode = false;
    
    // Set Text Fields
    
    elevationLeftText.text = [NSString stringWithFormat:@"%.0f°",elevationLeftSlider.value];
    elevationRightText.text = [NSString stringWithFormat:@"%.0f°",elevationRightSlider.value];
    distanceLeftText.text = [NSString stringWithFormat:@"%.0f m",distanceLeftSlider.value];
    distanceRightText.text = [NSString stringWithFormat:@"%.0f m",distanceRightSlider.value];
    
    [self initCircularSliders];
    [self initVerticalSliders];
    [self initPresetList];
    
    [distanceLeftSlider setTintColor:[UIColor blueColor]];
    [distanceRightSlider setTintColor:[UIColor redColor]];
//    distanceLeftSlider set
    
    // Set default values in Spatial Audio Unit
    [self.audioUnit setElevationLeftAngle:0.0];
    [self.audioUnit setElevationRightAngle:0.0];

    NSLog(@"Firing Up!");
    if (self.audioUnit==nil)
        return;
}



- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) beginRequestWithExtensionContext:(NSExtensionContext*)context
{
    [super beginRequestWithExtensionContext:context];
}


-(AUAudioUnit*) createAudioUnitWithComponentDescription:(AudioComponentDescription)componentDescription error:(NSError * _Nullable __autoreleasing * _Nullable)error
{
    self.audioUnit = [[SpatialAudioUnit alloc] initWithComponentDescription:componentDescription error:error];
    
    return _audioUnit;
}


// ---------- IBActions Methods --------------

// Azimuth

-(void)aziLeftChanged:(EFCircularSlider*)slider {

   [self updateAzimuthLeftLabel:slider.currentValue];
    float cookedValue = fabs(1.0 - slider.currentValue);
    [self.audioUnit setAzimuthLeftAngle:cookedValue];
}
-(void)aziRightChanged:(EFCircularSlider*)slider {
    [self updateAzimuthRightLabel:slider.currentValue];
    float cookedValue = fabs(1.0 - slider.currentValue);
    [self.audioUnit setAzimuthRightAngle:cookedValue];
}

// Elevation

- (IBAction)elevationLeftSliderChanged:(id)sender {
    [self updateElevationLeftLabel:elevationLeftSlider.value];
    [self.audioUnit setElevationLeftAngle:elevationLeftSlider.value];
}

- (IBAction)elevationRightSliderChanged:(id)sender {
    [self updateElevationRightLabel:elevationRightSlider.value];
    [self.audioUnit setElevationRightAngle:elevationRightSlider.value];
}

// Distance

- (IBAction)distanceLeftSliderChanged:(id)sender {
    [self.audioUnit setDistanceLeft:distanceLeftSlider.value];
    [self updateDistanceLeftLabel:distanceLeftSlider.value];
}


- (IBAction)distanceRightSliderChanged:(id)sender {
    [self.audioUnit setDistanceRight:distanceRightSlider.value];
    [self updateDistanceRightLabel:distanceRightSlider.value];

}

// HRTF Mode

- (IBAction)hrtfModeSwitched:(id)sender {
    [self.audioUnit toggleHRTFMode:!HRTFMode];
    if(HRTFMode)
        HRTFMode = false;
    else
        HRTFMode = true;
}

// Presets

- (IBAction)presetButtonPressed:(id)sender {
    if(presetTableView.hidden) {
        presetTableView.hidden = NO;
    }
    else
        presetTableView.hidden = YES;
}


// --------- Helper Methods -------------

// Update with raw 0 to 1 value

- (void)updateAzimuthLeft:(float)value {
    azimuthLeftCircularSlider.currentValue = value;
    [self aziLeftChanged:azimuthLeftCircularSlider];
}

- (void)updateAzimuthRight:(float)value {
    azimuthRightCircularSlider.currentValue = value;
    [self aziRightChanged:azimuthRightCircularSlider];
}

- (void)updateElevationLeft:(float)value {
    [elevationLeftSlider setValue:value];
    [self elevationLeftSliderChanged:elevationLeftSlider];
}

- (void)updateElevationRight:(float)value {
    [elevationRightSlider setValue:value];
    [self elevationRightSliderChanged:elevationRightSlider];
}

- (void)updateDistanceLeft:(float)value {
    [distanceLeftSlider setValue:value];
    [self distanceLeftSliderChanged:distanceLeftSlider];
}

- (void)updateDistanceRight:(float)value {
    [distanceRightSlider setValue:value];
    [self distanceRightSliderChanged:distanceRightSlider];
}

// Update with degrees

-(void)updateAzimuthLeftDegrees:(float)degrees {
    float cookedVal = degrees/360.0;
    [self updateAzimuthLeft:cookedVal];
}

-(void)updateAzimuthRightDegrees:(float)degrees {
    float cookedVal = degrees/360.0;
    [self updateAzimuthRight:cookedVal];
}

-(void)updateElevationLeftDegrees:(float)degrees {
    float cookedVal = (degrees+45)/120.0;
    
    [self updateElevationLeft:cookedVal];
}

-(void)updateElevationRightDegrees:(float)degrees {
    float cookedVal = (degrees+45)/120.0;
    [self updateElevationRight:cookedVal];
}


// Update Labels Methods

-(void)updateAzimuthLeftLabel:(float)value {
    float degrees = value * 360;
    if(degrees > 180)
        degrees = degrees - 360;
    aziLeftLabel.text = [NSString stringWithFormat:@"%.0f°",degrees];
}

-(void)updateAzimuthRightLabel:(float)value {
    float degrees = value * 360;
    if(degrees > 180)
        degrees = degrees - 360;
    aziRightLabel.text = [NSString stringWithFormat:@"%.0f°",degrees];
}

- (void)updateElevationLeftLabel:(float)value {
    float degrees = (value * 120) - 45;
    elevationLeftText.text = [NSString stringWithFormat:@"%.0f°",degrees];
}
- (void)updateElevationRightLabel:(float)value {
    float degrees = (value * 120) - 45;
    elevationRightText.text = [NSString stringWithFormat:@"%.0f°",degrees];
}
- (void)updateDistanceLeftLabel:(float)value {
    float distance = value * 1.95;
    distanceLeftText.text = [NSString stringWithFormat:@"%.0f m",distance];
}
- (void)updateDistanceRightLabel:(float)value {
    float distance = value * 1.95;
    distanceRightText.text = [NSString stringWithFormat:@"%.0f m",distance];
}

-(void)setPreset:(NSString *)preset {
    
    PresetType currentPreset = [self.presetData indexOfObject:preset];
    
    switch(currentPreset) {
        case PresetNearField:
            // 30° for L/R, Distance
            [self updateAzimuthLeftDegrees:330];
            [self updateAzimuthRightDegrees:30];
            [self updateElevationLeftDegrees:0];
            [self updateElevationRightDegrees:0];
            [self updateDistanceLeft:1];
            [self updateDistanceRight:1];
            break;
            
        case PresetFarField:
            // 30° forL/R, Elevation up
            [self updateAzimuthLeftDegrees:330];
            [self updateAzimuthRightDegrees:30];
            [self updateElevationLeftDegrees:45];
            [self updateElevationRightDegrees:45];
            [self updateDistanceLeft:2];
            [self updateDistanceRight:2];
            break;
            
        case PresetWideNearField:
            // 30° forL/R, Elevation up
            [self updateAzimuthLeftDegrees:315];
            [self updateAzimuthRightDegrees:45];
            [self updateElevationLeftDegrees:0];
            [self updateElevationRightDegrees:0];
            [self updateDistanceLeft:1];
            [self updateDistanceRight:1];
            break;
            
        case PresetReverseNearField:
            // 30° forL/R, Elevation up
            [self updateAzimuthLeftDegrees:150];
            [self updateAzimuthRightDegrees:210];
            [self updateElevationLeftDegrees:0];
            [self updateElevationRightDegrees:0];
            [self updateDistanceLeft:1];
            [self updateDistanceRight:1];
            break;
            
        case PresetFrontBack:
            // 0° for Left, 180° for Right
            [self updateAzimuthLeftDegrees:0];
            [self updateAzimuthRightDegrees:180];
            [self updateElevationLeftDegrees:0];
            [self updateElevationRightDegrees:0];
            [self updateDistanceLeft:2];
            [self updateDistanceRight:2];
            break;
            
        case PresetAbove:
            [self updateAzimuthLeftDegrees:270];
            [self updateAzimuthRightDegrees:90];
            [self updateElevationLeftDegrees:75];
            [self updateElevationRightDegrees:75];
            [self updateDistanceLeft:2];
            [self updateDistanceRight:2];
            break;
            
        case PresetBelow:
            [self updateAzimuthLeftDegrees:270];
            [self updateAzimuthRightDegrees:90];
            [self updateElevationLeftDegrees:-45];
            [self updateElevationRightDegrees:-45];
            [self updateDistanceLeft:2];
            [self updateDistanceRight:2];
            break;
            
        case PresetNone:
            [self updateAzimuthLeftDegrees:0];
            [self updateAzimuthRightDegrees:0];
            [self updateElevationLeftDegrees:0];
            [self updateElevationRightDegrees:0];
            [self updateDistanceLeft:1];
            [self updateDistanceRight:1];
            break;
            
        default:
            break;
    }
}

#pragma mark - Init Helper Methods


-(void)initCircularSliders {
    // EF Circular Slider setup
    [azimuthLeftCircularSlider addTarget:self action:@selector(aziLeftChanged:) forControlEvents:UIControlEventValueChanged];
    [self.view addSubview:azimuthLeftCircularSlider];
    [azimuthLeftCircularSlider setCurrentValue:0.0f];
    
    [azimuthRightCircularSlider addTarget:self action:@selector(aziRightChanged:) forControlEvents:UIControlEventValueChanged];
    [self.view addSubview:azimuthRightCircularSlider];
    [azimuthRightCircularSlider setCurrentValue:0.0f];
    
    [azimuthLeftCircularSlider setLineWidth:7];
    [azimuthRightCircularSlider setLineWidth:7];

    
    // Initial values for Circular Slider
    azimuthLeftCircularSlider.maximumValue = 1.0;
    azimuthLeftCircularSlider.minimumValue = 0.0;
    azimuthLeftCircularSlider.currentValue = 0.0;
    azimuthRightCircularSlider.maximumValue = 1.0;
    azimuthRightCircularSlider.minimumValue = 0.0;
    azimuthRightCircularSlider.currentValue = 0.0;
    // Set text labels for circular sliders
    aziLeftLabel.text = [NSString stringWithFormat:@"%.0f°", azimuthLeftCircularSlider.currentValue];
    aziRightLabel.text = [NSString stringWithFormat:@"%.0f°", azimuthRightCircularSlider.currentValue];
    // Customize appearance of circules sliders
    azimuthLeftCircularSlider.handleType = EFDoubleCircleWithClosedCenter;
    azimuthRightCircularSlider.handleType = EFDoubleCircleWithClosedCenter;
    azimuthLeftCircularSlider.filledColor = [UIColor blueColor];
    azimuthLeftCircularSlider.unfilledColor = [UIColor blueColor];
    azimuthRightCircularSlider.filledColor = [UIColor redColor];
    azimuthRightCircularSlider.unfilledColor = [UIColor redColor];
    azimuthLeftCircularSlider.handleColor = [UIColor blackColor];
    azimuthRightCircularSlider.handleColor = [UIColor blackColor];
}

-(void)initVerticalSliders {
    // Setup Vertical Slider
    [elevationLeftSlider setThumbImage:[UIImage imageNamed:@"SliderThumb"] forState:UIControlStateNormal];
    [elevationRightSlider setThumbImage:[UIImage imageNamed:@"SliderThumb"] forState:UIControlStateNormal];
    [elevationLeftSlider setTintColor:[UIColor blueColor]];
    [elevationRightSlider setTintColor:[UIColor redColor]];
    [self updateElevationLeftDegrees:0];
    [self updateElevationRightDegrees:0];
}

-(void)initPresetList {
    // Setup for Preset List
    self.presetData = [[NSArray alloc] initWithObjects:@"Near Field Monitors",@"Wide Near Field Monitors",@"Reverse Near Field Monitors",@"Far Field Monitors",@"Front and Rear", @"Above", @"Below",@"None",nil];
    presetTableView.delegate = self;
    presetTableView.dataSource = self;
    presetTableView.hidden = YES;
    [presetButton setTitle:@"Presets" forState:UIControlStateNormal];
}

#pragma mark - UITableView Datasource Methods
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.presetData count];

}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *tableIdentifier = @"Presets";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableIdentifier];
    
    if(cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableIdentifier];
    }
    
    cell.textLabel.text = [self.presetData objectAtIndex:indexPath.row];
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [presetTableView cellForRowAtIndexPath:indexPath];
    if([cell.textLabel.text isEqualToString:@"None"])
        [presetButton setTitle:@"Presets" forState:UIControlStateNormal];
    else
        [presetButton setTitle:cell.textLabel.text forState:UIControlStateNormal];
    
    [self setPreset:cell.textLabel.text];
    
    presetTableView.hidden = YES;
}

@end
