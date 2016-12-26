/*
  ==============================================================================

    PSMoveThread.h
    Created: 25 Dec 2016 3:12:38pm
    Author:  Tim Arterbury

  ==============================================================================
*/

#ifndef PSMOVETHREAD_H_INCLUDED
#define PSMOVETHREAD_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class PSMoveThread : public Thread
{
    
public:
    PSMoveThread();
    ~PSMoveThread();
    void run() override;
    
private:
    ScopedPointer<MidiOutput> midiOutput;
    MidiBuffer midiBuffer;
    
    
};



#endif  // PSMOVETHREAD_H_INCLUDED
