/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "psmove.h"
#include "psmove_tracker.h"
// Private header for psmove_port_sleep_ms()
#include "psmove_port.h"

#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"

#include "TrackingToggle.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   :  public OpenGLAppComponent,
                                public HighResolutionTimer
{
public:
    //==============================================================================
    MainContentComponent()
    //: psMoveThread("PSMoveDataProccessing") // 2nd parameter defaults to default
                                                // Mac OSX stack size. This could be
                                                // modified later for thread
                                                // optimization
    :
    // Allocates the number of controllers in a heap block
    aControllerIsConnected(false),
    controllers(controllerCount = psmove_count_connected())
    /*,
    controllersTrackingToggle(controllerCount)*/
    
    {
    
        
    
        /*
        trackingToggle1.initializeWithId(0);
        addAndMakeVisible(trackingToggle1);
        
        trackingToggle2.initializeWithId(1);
        addAndMakeVisible(trackingToggle2);
         */
        
        
        // Check Correct Library Version
        if (!psmove_init(PSMOVE_CURRENT_VERSION))
        {
            std::cerr << "PS Move API init failed (wrong version?)\n";
            exit(1);
        }
     
        // Output the Number of Controllers Allocated For
        if (controllerCount > 0)
        {
            std::cout << "#### Found " << controllerCount << " controllers connected via Bluetooth.\n";
            aControllerIsConnected = true;
        }
        else
        {
            std::cout << "No controller's are connected via Bluetooth.";
        }
        
        
        // Create MIDI Output Device
        midiOutput = MidiOutput::createNewDevice("PSMove");
        
        if (midiOutput == NULL)
            std::cerr << "Failure to create MidiOutput stream.";
        
        //midiOutput->startBackgroundThread();
    
        
        // Process PS Move Data
        if (aControllerIsConnected)
        {
            // Intitializes Tracker and Move Controller
            initializePSMoveSystem();
            
            // Trigger PSMove rumble and lights to signal initialization has occured

            for (int i = 0; i < controllerCount; ++i)
            {
                for (int j = 0; j < 10; j++) {
                    psmove_set_leds(controllers[i], 0, 255 * (j % 3 == 0), 0);
                    psmove_set_rumble(controllers[i], 255 * (j % 2));
                    psmove_update_leds(controllers[i]);
                    psmove_port_sleep_ms(10 * (j % 10));
                }
                
                for (int j = 250; j >= 0; j -= 5) {
                    psmove_set_leds(controllers[i], (unsigned char) j, (unsigned char) j, 0);
                    psmove_set_rumble(controllers[i], 0);
                    psmove_update_leds(controllers[i]);
                }
            }
            
            std::cout << "Controller Count: " << controllerCount << '\n';
            
            // Setup GUI
            // Allocate memory for TrackingToggles
            for (int i = 0; i < controllerCount; ++i)
            {
                trackingToggles.add(new TrackingToggle());
            }
            
            // Initialize and add to this component
            for (int i = 0; i < controllerCount; ++i)
            {
                trackingToggles[i]->initializeWithId(i);
                addAndMakeVisible(trackingToggles[i]);
            }
            
            // This access throws a BAD ACCESS ???
            // AM I USING HEAPBLOCK WRONG???
            // With new() delete, no bad access is thrown, but the GUI will not
            // show up at all??
            //controllersTrackingToggle[0].initializeWithId(0);
            
            // Begin Processing PSMove & Tracker Data (repeatedly calls hiResTimerCallback()
            startTimer(1); // 1 milisecond intervals
        }
        
        
        
        // Setup Window (call this last because it calls resize()), we must
        // allocate memory for the GUI elements before calling rezise
        setSize (800, 600);
        
        
        // OLD VERSION
        //psMoveThread(10); // Give this thread highest priority over
                                          // visualizations or anything else
        
    }

    ~MainContentComponent()
    {
        //psMoveThread();
        
        stopTimer();
        
        // Disconnect the PsMove
        if (aControllerIsConnected)
        {
            // Disconnect all controllers
            for (int i = 0; i < controllerCount; ++i)
            {
                psmove_disconnect(controllers[i]);
            }
            psmove_tracker_free(tracker);
            aControllerIsConnected = false;
        }
        
        // Free the Heap Blocks (apparently this is unneeded)
        //controllers.free();
        //controllersTrackingToggle.free();
        
        shutdownOpenGL();
    }
    
    
    //==============================================================================
    // PSMove Functions
    
    void initializePSMoveSystem()
    {
        // Initialize Tracker
        std::cout << "Trying to init PSMoveTracker...";
        psmove_tracker_settings_set_default(&trackerSettings);
        trackerSettings.color_mapping_max_age = 0;
        trackerSettings.exposure_mode = Exposure_LOW;
        trackerSettings.camera_mirror = PSMove_True;
        tracker = psmove_tracker_new_with_settings(&trackerSettings);
        if (tracker == NULL)
        {
            std::cerr << "Could not init PSMoveTracker.\n";
            exit (1);
        }
        std::cout << "Tracker Initialized\n";
        
        
        // Sync Controllers
        for(int i = 0; i < controllerCount; ++i)
        {
            // Connect to controller
            std::cout << "Opening controller " << i << '\n';
            controllers[i] = psmove_connect_by_id(i);
            assert(controllers[i] != NULL); // This would be a PSMove library malfunction
            
            
            char *serial = psmove_get_serial(controllers[i]);
            std::cout << "Serial: " << serial << '\n';
            free (serial);
            
            psMoveconnectionType = psmove_connection_type(controllers[i]);
            switch (psMoveconnectionType)
            {
                case Conn_USB:
                    std::cout << "Connected via USB.\n";
                    break;
                case Conn_Bluetooth:
                    std::cout << "Connected via Bluetooth.\n";
                    break;
                case Conn_Unknown:
                    std::cout << "Unknown connection type.\n";
                    break;
            }
            
            // Calibrate controller for tracking
            while(true)
            {
                std::cout << "Calibrating controller " << i << "...\n";
                trackerCalibrated = psmove_tracker_enable(tracker, controllers[i]);
                
                if (trackerCalibrated == Tracker_CALIBRATED) {
                    enum PSMove_Bool auto_update_leds = psmove_tracker_get_auto_update_leds(tracker, controllers[i]);
                    std::cout << "OK, auto_update_leds is ";
                    if (auto_update_leds == PSMove_True)
                        std::cout << "enabled\n";
                    else
                        std::cout << "disabled\n";
                    break;
                } else {
                    std::cout << "ERROR - retrying\n";
                }
            }
        }
    }
    
    // Data Processing
    void hiResTimerCallback() override
    {
        // MIDI MESSAGE TEST
        //const MidiMessage swag(293, 232, 3432);
        //midiOutput->sendMessageNow(swag);

        psmove_tracker_update_image(tracker);
        psmove_tracker_update(tracker, NULL);
        psmove_tracker_annotate(tracker);
        
        //frame = psmove_tracker_get_frame(tracker);
        //if (frame) {
        //    cvShowImage("live camera feed", frame);
        //}
        
        for (int i = 0; i < controllerCount; ++i)
        {
            float fX, fY, fR;
            psmove_tracker_get_position(tracker, controllers[i], &fX, &fY, &fR);
            
            // Map the x position to an MIDI value from 0 to 127
            float temp;
            
            temp = jmap((float) fX, (float) 0, (float) 638, (float) 0, (float) 127);
            xTrack = (int)temp;
            
            temp = jmap((float) fY, (float) 0, (float) 480, (float) 0, (float) 127);
            yTrack = (int)temp;
            yTrack = 127 - yTrack; // Invert y position
            
            temp = jmap((float) fR, (float) 0, (float) 168, (float) 0, (float) 127);
            rTrack = (int)temp;
            
            
            // Output MIDI Data
            
            if (trackingToggles[i]->shouldTrackX())
                midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + (i * 3), xTrack));
            if (trackingToggles[i]->shouldTrackY())
                midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + 1 + (i * 3), yTrack));
            if (trackingToggles[i]->shouldTrackR())
                midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + 2 + (i * 3), rTrack));
            
            /*
            if (i == 0)
            {
                if (trackingToggle1.shouldTrackX())
                    midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + (i * 3), xTrack));
                if (trackingToggle1.shouldTrackY())
                    midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + 1 + (i * 3), yTrack));
                if (trackingToggle1.shouldTrackR())
                    midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + 2 + (i * 3), rTrack));
                
            }
            else if (i == 1)
            {
                if (trackingToggle2.shouldTrackX())
                    midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + (i * 3), xTrack));
                if (trackingToggle2.shouldTrackY())
                    midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + 1 + (i * 3), yTrack));
                if (trackingToggle2.shouldTrackR())
                    midiOutput->sendMessageNow(MidiMessage::controllerEvent(1, startMapCC + 2 + (i * 3), rTrack));
                
                
            }
            else
            {
            }
             */
            
            
            
        }
        //std::cout << xTrack << "   " << yTrack << "   " << rTrack << '\n';
    }
    
    //==============================================================================
    // OpenGL Functions
    void initialise() override
    {
    }

    void shutdown() override
    {
    }

    void render() override
    {
        OpenGLHelpers::clear (Colours::black);

    }

    //==============================================================================
    // GUI Functions
    void paint (Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.
        
        
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        
        
        //Rectangle<int> buttonsBounds (getLocalBounds().withWidth (halfWidth).reduced (10));
        
        Rectangle<int> trackingToggleBound(getWidth() / 3, 90);
        
        
        if (aControllerIsConnected)
        {
            for (int i = 0; i < controllerCount; ++i)
            {
                trackingToggles[i]->setBounds (trackingToggleBound.reduced(10));
                trackingToggleBound.translate(0, 90);
            }
        }
        
        /*
        
        trackingToggle1.setBounds (trackingToggleBound.reduced(10));
        trackingToggleBound.translate(0, 90);
        trackingToggle2.setBounds (trackingToggleBound.reduced(10));
        */
        
    }
    
    
private:
    //==============================================================================

    // private member variables
    //Thread psMoveThread;
    
    const int startMapCC = 24;
    
    // MIDI Output
    ScopedPointer<MidiOutput> midiOutput;
    MidiBuffer midiBuffer;
    
    // PSMove Controllers
    int controllerCount;
    bool aControllerIsConnected;
    HeapBlock<PSMove *> controllers;
    enum PSMove_Connection_Type psMoveconnectionType;
    
    // PSMove Tracker (PSEye)
    PSMoveTracker * tracker; // Scoped Pointer may not be used here because of
                             // how the PSMove library defines a PSMoveTracker
    PSMoveTrackerSettings trackerSettings;
    //void * frame;
    int trackerCalibrated;
    
    int xTrack, yTrack, rTrack;
    
    // FOR SOME REASON THIS KEEPS ERRORING OUT, I HAVE NO IDEA
    //HeapBlock<TrackingToggle, true> controllersTrackingToggle;
    
    OwnedArray<TrackingToggle> trackingToggles;
    
    //TrackingToggle trackingToggle1;
    //TrackingToggle trackingToggle2;
    
    
    //TrackingToggle firstControllerToggle;
    
    
    /*
    TextButton trackXButton, trackYButton, trackRButton;
    
    bool trackX, trackY, trackR;
     */
    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
