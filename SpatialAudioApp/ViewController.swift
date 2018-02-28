/*
	Copyright (C) 2015 Apple Inc. All Rights Reserved.
	See LICENSE.txt for this sample’s licensing information
	
	Abstract:
	View controller which registers an AUAudioUnit subclass in-process for easy development, connects sliders and text fields to its parameters, and embeds the audio unit's view into a subview. Uses SimplePlayEngine to audition the effect.
*/

import UIKit
import AudioToolbox
import SpatialAppFramework


class ViewController: UIViewController {
    // MARK: Properties

	@IBOutlet var playButton: UIButton!

	    /// Container for our custom view.
    @IBOutlet var auContainerView: UIView!

	/// The audio playback engine.
	var playEngine: SimplePlayEngine!


	/// Our plug-in's custom view controller. We embed its view into `viewContainer`.
	var spatialAppViewController: SpatialAppViewController!

    // MARK: View Life Cycle
    
	override func viewDidLoad() {
		super.viewDidLoad()
		
		// Set up the plug-in's custom view.
		embedPlugInView()
		
		// Create an audio file playback engine.
		playEngine = SimplePlayEngine()
		
		/*
			Register the AU in-process for development/debugging.
			First, build an AudioComponentDescription matching the one in our 
            .appex's Info.plist.
		*/
        var componentDescription = AudioComponentDescription()
        componentDescription.componentType = kAudioUnitType_Effect
        // sptl
        componentDescription.componentSubType = 0x7370746c
        componentDescription.componentManufacturer = 0x47484155
        componentDescription.componentFlags = 0
        componentDescription.componentFlagsMask = 0
		
		/*
			Register our `AUAudioUnit` subclass, `AUv3FilterDemo`, to make it able 
            to be instantiated via its component description.
			
			Note that this registration is local to this process.
		*/
        AUAudioUnit.registerSubclass(SpatialAudioUnit.self, as: componentDescription, name: "Local SpatialApp", version: UInt32.max)

		// Instantiate and insert our audio unit effect into the chain.
		playEngine.selectEffectWithComponentDescription(componentDescription) {
			// This is an asynchronous callback when complete. Finish audio unit setup.
			self.connectParametersToControls()
		}
	}
	
	/// Called from `viewDidLoad(_:)` to embed the plug-in's view into the app's view.
	func embedPlugInView() {
        /*
			Locate the app extension's bundle, in the app bundle's PlugIns
			subdirectory. Load its MainInterface storyboard, and obtain the
            `FilterDemoViewController` from that.
        */
        let builtInPlugInsURL = Bundle.main.builtInPlugInsURL!
        let pluginURL = builtInPlugInsURL.appendingPathComponent("SpatialAppExtension.appex")
		let appExtensionBundle = Bundle(url: pluginURL)

        let storyboard = UIStoryboard(name: "MainInterface", bundle: appExtensionBundle)

        
		spatialAppViewController = storyboard.instantiateInitialViewController() as! SpatialAppViewController
        
        // Present the view controller's view.
        if let view = spatialAppViewController.view {
            addChildViewController(spatialAppViewController)
            view.frame = auContainerView.bounds
            
            auContainerView.addSubview(view)
            spatialAppViewController.didMove(toParentViewController: self)
        }
	}
	
	/**
        Called after instantiating our audio unit, to find the AU's parameters and
        connect them to our controls.
    */
	func connectParametersToControls() {

        let audioUnit = playEngine.audioUnit as! SpatialAudioUnit
        spatialAppViewController.audioUnit = audioUnit

	}


    // MARK: IBActions

	/// Handles Play/Stop button touches.
    @IBAction func togglePlay(_ sender: AnyObject?) {
		let isPlaying = playEngine.togglePlay()

        let titleText = isPlaying ? "Stop" : "Play"

		playButton.setTitle(titleText, for: UIControlState())
	}
	

}
