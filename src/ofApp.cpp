#include "ofApp.h"
#include "ShapeState.h"
#include "SketchState.h"

//--------------------------------------------------------------
void ofApp::setup(){   
    ofxEasyFft& fft = _stateMachine.getSharedData();
    fft.setup(16384);
    fft.setUseNormalization(false);
    
    _stateMachine.addState<ShapeState>();
    _stateMachine.addState<SketchState>();

    _states.push_back("Shapes");
    _states.push_back("Sketches");
    
    _stateIndex = 0;
    _stateMachine.changeState(_states[_stateIndex]);
}

//--------------------------------------------------------------
void ofApp::update(){
    ofxEasyFft& fft = _stateMachine.getSharedData();
    fft.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'f') {
        ofToggleFullscreen();
    } else if (key == OF_KEY_DOWN) {
        _stateIndex = (_stateIndex + 1) % _states.size();
        _stateMachine.changeState(_states[_stateIndex]);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
