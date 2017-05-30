#pragma once

#include "ofMain.h"
#include "ofxEasyFft.h"
#include "ofxPostProcessing.h"

#include "ofxTween.h"
#include "ofxStateMachine.h"

using namespace itg;

class ofApp : public ofBaseApp {

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

private:
    ofxStateMachine<ofxEasyFft> _stateMachine;
    vector<string> _states;
    int _stateIndex;
};
