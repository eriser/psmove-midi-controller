//
//  PSMoveFunctions.h
//  PSMove Midi Controller
//
//  Created by Tim Arterbury on 12/25/16.
//
//

#ifndef PSMoveFunctions_h
#define PSMoveFunctions_h

#include "psmove.h"
#include "psmove_tracker.h"
// Private header for psmove_port_sleep_ms()
#include "psmove_port.h"

#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"

void initializePSMoveSystem()
{

    controllerIsConnected = false;

    // Initialize API
    if (!psmove_init(PSMOVE_CURRENT_VERSION))
    {
        std::cerr << "PS Move API init failed (wrong version?)\n";
        exit(1);
    }
        
    // Count Controllers
    int controllerCount = psmove_count_connected();
    std::cout << "Connected controllers: " << controllerCount << "\n";
    
    // Initialize Tracker
    fprintf(stderr, "Trying to init PSMoveTracker...");
    psmove_tracker_settings_set_default(&trackerSettings);
    trackerSettings.color_mapping_max_age = 0;
    trackerSettings.exposure_mode = Exposure_LOW;
    trackerSettings.camera_mirror = PSMove_True;
    tracker = psmove_tracker_new_with_settings(&trackerSettings);
    if (!tracker)
    {
        fprintf(stderr, "Could not init PSMoveTracker.\n");
        exit (1);
    }
    fprintf(stderr, "Tracker Initialized\n");
    
    // Connect Controllers
    printf("Opening controller 1\n");
    controller = psmove_connect();
    
    // When I implement for more controllers use this in a loop with an array of
    // controllers
    //controllers[i] = psmove_connect_by_id(i);
    //assert(controllers[i] != NULL);
    
    // Callibrate Controllers
    for(;;)
    {
        trackerCalibrated = psmove_tracker_enable(tracker, controller);
        
        if (trackerCalibrated == Tracker_CALIBRATED) {
            enum PSMove_Bool auto_update_leds = psmove_tracker_get_auto_update_leds(tracker, controller);
            printf("OK, auto_update_leds is %s\n",
                   (auto_update_leds == PSMove_True)?"enabled":"disabled");
            break;
        } else {
            printf("ERROR - retrying\n");
        }
    }
    
    // In the above loop for all the controllers in the future
    //printf("Calibrating controller %d...", i);
    //fflush(stdout);
    //trackerCalibrated = psmove_tracker_enable(tracker, controllers[i]);
    
    
    if (controller == NULL)
    std::cout << "Could not connect to default Move controller.\n"
    "Please connect one via USB or Bluetooth.\n";
    else
    controllerIsConnected = true;
    
    
    if (controllerIsConnected)
    {
        char *serial = psmove_get_serial(controller);
        std::cout << "Serial: %s\n" << serial;
        free(serial);
        
        ctype = psmove_connection_type(controller);
        switch (ctype) {
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
    }
}



#endif /* PSMoveFunctions_h */
