//
//  TrackingToggle.h
//  PSMove Midi Controller
//
//  Created by Tim Arterbury on 12/26/16.
//
//

#ifndef TrackingToggle_h
#define TrackingToggle_h

#include "../JuceLibraryCode/JuceHeader.h"

// Combines all toggling functionality (including buttons and if the data
// should be sent into one object).
class TrackingToggle :  public Component,
                        private Button::Listener
{
public:
    TrackingToggle()
    {
        controllerId = -1;
        
        addAndMakeVisible (trackXButton);
        trackXButton.addListener (this);
        trackX = false;
        
        addAndMakeVisible (trackYButton);
        trackYButton.addListener (this);
        trackY = false;
        
        addAndMakeVisible (trackRButton);
        trackRButton.addListener (this);
        trackR = false;
    }
    
    void initializeWithId (int id)
    {
        controllerId = id;
        
        String idString(id);
        String title;
        
        title = "Controller " + idString + " - Track X";
        trackXButton.setButtonText (title);
        trackYButton.setButtonText (title.replaceSection ((title.length() - 1), 1, "Y"));
        trackRButton.setButtonText (title.replaceSection ((title.length() - 1), 1, "R"));
    }
    
    bool shouldTrackX()
    {
        return trackX;
    }
    
    bool shouldTrackY()
    {
        return trackY;
    }
    
    bool shouldTrackR()
    {
        return trackR;
    }
    
    
    void buttonClicked (Button* button) override
    {
        if (button == &trackXButton)        trackX = !trackX;
        if (button == &trackYButton)        trackY = !trackY;
        if (button == &trackRButton)        trackR = !trackR;
    }
    
    void resized() override
    {
        int thirdHeight = getHeight() / 3;
        Rectangle<int> rectBound(getWidth(), thirdHeight);
        
        trackXButton.setBounds (rectBound);
        trackYButton.setBounds (rectBound = rectBound.translated(0, thirdHeight));
        trackRButton.setBounds (rectBound = rectBound.translated(0, thirdHeight));
    }
    
    void paint (Graphics& g) override
    {
    }
    
private:
    int controllerId;
    TextButton trackXButton, trackYButton, trackRButton;
    bool trackX, trackY, trackR;
};


#endif /* TrackingToggle_h */
