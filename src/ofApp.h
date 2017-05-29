#pragma once

#include "ofMain.h"
#include "ofxEasyFft.h"
#include "ofxPostProcessing.h"

#include "ofxTween.h"

#define N_POLYS 4

class ofApp : public ofBaseApp{

public:
    
    enum Mode {
        CircleSingle,
        CircleMulti,
        Polygon,
        Typography
    };
    
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
    bool _useMean;
    bool _autoFill;
    
    Mode _mode;
    ofCamera _cam[3];
    ofEasyCam _easyCam;
    
    ofxEasyFft _fft;
    ofPolyline _polys[N_POLYS], _chars[N_POLYS], _shapes[N_POLYS];
    ofMesh _meshes[N_POLYS];
    float _mean[N_POLYS], _max[N_POLYS];
    int _nVerts[N_POLYS] = {3, 4, 5, 6};
    
    ofTessellator _tessellator;
    int _nBuffers;
    
    ofImage _bg;
    ofxPostProcessing _post;
    
    ofTrueTypeFont _font;
    
    ofxTween _posTween, _scaleTween, _alphaTween[N_POLYS];
    ofxEasingLinear _linear;
    ofxEasingCubic _cubic;
    ofxEasingExpo _expo;
    
    void setupPolygons();
    void setupText();
};
