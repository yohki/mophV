#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    _nBuffers = 1024;
    for (int i = 0; i < N_POLYS; i++) {
        if (i == 0) {
            _posTween.setParameters(_linear, ofxTween::easeInOut, 0, 0, 0, 0);
            _scaleTween.setParameters(_linear, ofxTween::easeInOut, 1, 1, 0, 0);
        } else {
            _posTween.addValue(0, 0);
            _scaleTween.addValue(1, 1);
        }
        _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, 0, 0, 0, 0);
        _mean[i] = 0;
    }
    _fft.setup(16384);
    
    _post.init(ofGetWidth(), ofGetHeight());
    _post.createPass<FxaaPass>()->setEnabled(true);
    _post.createPass<BloomPass>()->setEnabled(true);

    _font.load("CalvertMTStd.otf", 300, true, true, true);
    _mode = CircleSingle;
    _useMean = true;
    _autoFill = false;
}

//--------------------------------------------------------------
void ofApp::update(){
    _fft.update();
    
    _posTween.update();
    _scaleTween.update();
    for (int i = 0; i < N_POLYS; i++) {
        _alphaTween[i].update();
    }
    
    vector<float> buffer = _fft.getBins();
    int nPoints = _nBuffers / N_POLYS;
    
    float fillAlpha = 128;
    
    switch (_mode) {
        case CircleSingle:
        case CircleMulti:
            for (int i = 0; i < N_POLYS; i++) {
                float mean = 0;
                float max = 0;
                
                _polys[i].clear();
                for (int j = 0; j < nPoints; j++) {
                    int index = i * nPoints + j;
                    float rad = TWO_PI / nPoints * j;
                    float dr = (isnan(buffer[index]) ? 0 : buffer[index] * 100);
                    float r = dr + 100;
                    float x = r * cos(rad);
                    float y = r * sin(rad);
                    _polys[i].addVertex(ofVec3f(x, y, 0));
                    
                    if (!isnan(buffer[index])) {
                        mean += buffer[index];
                        max = MAX(buffer[index], max);
                    }
                }
                _polys[i].close();
                _tessellator.tessellateToMesh(_polys[i], OF_POLY_WINDING_ODD, _meshes[i]);

                mean /= nPoints;
                if (_autoFill) {
                    if (_useMean) {
                        if (1.25 < mean / _mean[i]) {
                            _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, fillAlpha, 0, 250, 0);
                        }
                    } else {
                        if (1.5 < max / _max[i]) {
                            _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, fillAlpha, 0, 250, 0);
                        }
                    }
                }
                _mean[i] = mean;
                _max[i] = max;
            }
            break;
        case Polygon:
            for (int i = 0; i < N_POLYS; i++) {
                float mean = 0;
                float max = 0;
                
                _polys[i].clear();
                for (int j = 0; j < _shapes[i].size(); j++) {
                    int index = i * nPoints + j;
                    ofVec3f norm = _shapes[i].getNormalAtIndex(j);
                    float d = (isnan(buffer[index])? 0 : buffer[index] * 100);
                    ofVec3f v(_shapes[i][j].x + norm.x * d, _shapes[i][j].y + norm.y * d, 0);
                    _polys[i].addVertex(v);
                    
                    if (!isnan(buffer[index])) {
                        mean += buffer[index];
                        max = MAX(buffer[index], max);
                    }
                }
                _polys[i].close();
                _tessellator.tessellateToMesh(_polys[i], OF_POLY_WINDING_NONZERO, _meshes[i]);
                
                mean /= nPoints;
                if (_autoFill) {
                    if (_useMean) {
                        if (1.25 < mean / _mean[i]) {
                            _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, fillAlpha, 0, 250, 0);
                        }
                    } else {
                        if (1.5 < max / _max[i]) {
                            _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, fillAlpha, 0, 250, 0);
                        }
                    }
                }
                _mean[i] = mean;
                _max[i] = max;
            }
            break;
        case Typography:
            for (int i = 0; i < N_POLYS; i++) {
                float mean = 0;
                float max = 0;
                
                for (int j = 0; j < nPoints; j++) {
                    int index = i * nPoints + j;
                    if (!isnan(buffer[index])) {
                        mean += buffer[index];
                        max = MAX(buffer[index], max);
                    }
                }
                mean /= nPoints;
                if (_autoFill) {
                    if (_useMean) {
                        if (1.25 < mean / _mean[i]) {
                            _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, fillAlpha, 0, 250, 0);
                        }
                    } else {
                        if (1.5 < max / _max[i]) {
                            _alphaTween[i].setParameters(_linear, ofxTween::easeInOut, fillAlpha, 0, 250, 0);
                        }
                    }
                }
                _mean[i] = mean;
                _max[i] = max;

                _polys[i].clear();

                int smooth = ofMap(_useMean ? mean : max, 0, 1, 1, 150);
                _polys[i] = _chars[i].getSmoothed(smooth, 0.0);
                _tessellator.tessellateToMesh(_polys[i], OF_POLY_WINDING_NONZERO, _meshes[i]);
                
            }

            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    _post.begin(_easyCam);
    ofBackground(0);
    ofSetColor(255);
    
    ofPushMatrix();
    
    switch(_mode) {
        case CircleSingle:
        case CircleMulti:
        case Polygon:
        case Typography:
            for (int i = 0; i < N_POLYS; i++) {
                ofPushMatrix();
                float x = _posTween.getTarget(i);
                ofTranslate(x, 0);
                float angle = (i % 2 == 0 ? 1.0 : -1.0) * fmod(ofGetElapsedTimef() * 3.0, 360);
                ofRotateZ(angle);
                if (_mode == Typography) {
                    ofRotateX(angle);
                }
                float scale = _scaleTween.getTarget(i);
                ofScale(scale, scale);
                
                float alpha = _alphaTween[i].getTarget(0);
                ofSetColor(255, alpha);
                _meshes[i].draw();
                ofSetColor(255, 192);
                _polys[i].draw();
                if (_mode == Typography) {
                    ofSetColor(255, 64);
                    _chars[i].draw();
                }
                ofPopMatrix();
            }
            break;
        default:
            break;
    }
    
    ofPopMatrix();
    
    _post.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        if (_mode == CircleSingle) {
            _mode = CircleMulti;
            _autoFill = true;
            for (int i = 0; i < N_POLYS; i++) {
                float x = ofGetWidth() / N_POLYS * (i + 0.5) - ofGetWidth() * 0.5;
                if (i == 0) {
                    _posTween.setParameters(_linear, ofxTween::easeInOut, 0, x, 25000, 0);
                    _scaleTween.setParameters(_linear, ofxTween::easeInOut, 1.0, 0.7, 25000, 0);
                } else {
                    _posTween.addValue(0, x);
                    _scaleTween.addValue(1.0, 0.7);
                }
            }
        } else if (_mode == CircleMulti){
            _mode = Polygon;
            setupPolygons();
        } else if (_mode == Polygon) {
            _mode = Typography;
            setupText();
        } else if (_mode == Typography){
            _mode = CircleSingle;
            _autoFill = false;
            for (int i = 0; i < N_POLYS; i++) {
                float x = ofGetWidth() / N_POLYS * (i + 0.5) - ofGetWidth() * 0.5;
                if (i == 0) {
                    _posTween.setParameters(_expo, ofxTween::easeInOut, x, 0, 500, 0);
                    _scaleTween.setParameters(_expo, ofxTween::easeInOut, 0.7, 1.0, 500, 0);
                } else {
                    _posTween.addValue(x, 0);
                    _scaleTween.addValue(0.7, 1.0);
                }
            }
        }
    }
}

void ofApp::setupPolygons() {
    for (int i = 0; i < N_POLYS; i++) {
        _shapes[i].clear();
        int n = _nBuffers / N_POLYS / _nVerts[i];

        for (int j = 0; j < _nVerts[i]; j++) {
            float radStart = TWO_PI / _nVerts[i] * j;
            float radEnd = TWO_PI / _nVerts[i] * ((j + 1) % _nVerts[i]);
            
            float r = 100;
            ofPoint from(r * cos(radStart), r * sin(radStart));
            ofPoint to(r * cos(radEnd), r * sin(radEnd));
            for (int k = 0; k < n; k++) {
                float p = 1.0 * k / n;
                ofPoint v = from.interpolate(to, p);
                _shapes[i].addVertex(v);
            }
        }
        _shapes[i].close();
        _polys[i] = _shapes[i].getResampledByCount(_nBuffers / N_POLYS);
        _tessellator.tessellateToMesh(_polys[i], OF_POLY_WINDING_NONZERO, _meshes[i]);
    }
}

void ofApp::setupText() {
    string text = "moph";
    
    for (int i = 0; i < N_POLYS; i++) {
        ofTTFCharacter path = _font.getStringAsPoints(text)[i];
        vector<ofPolyline> polylines = path.getOutline();
        
        int n = polylines.size();
        int pathIndex = 0;
        float area = 0;
        for (int j = 0; j < n; j++) {
            float a = polylines[j].getBoundingBox().getArea();
            if (area < a) {
                pathIndex = j;
                area = a;
            }
        }
        auto poly = polylines[pathIndex].getResampledByCount(_nBuffers / N_POLYS);
        ofRectangle bb = poly.getBoundingBox();

        float cx = bb.x + bb.width * 0.5;
        float cy = bb.y + bb.height * 0.5;

        _chars[i].clear();
        for (int j = 0; j < poly.size(); j++) {
            _chars[i].addVertex(poly[j].x - cx, poly[j].y - cy);
        }
        _chars[i].close();
        _tessellator.tessellateToMesh(_chars[i], OF_POLY_WINDING_NONZERO, _meshes[i]);
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
