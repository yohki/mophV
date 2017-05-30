#include "ofxState.h"
#include "ofxEasyFft.h"
#include "ofxTween.h"
#include "ofxJSON.h"
#include "ofxBox2d.h"

static bool removeShapeOffScreen(shared_ptr<ofxBox2dBaseShape> shape) {
    if (!ofRectangle(0, -200, ofGetWidth(), ofGetHeight() + 200).inside(shape.get()->getPosition())) {
        return true;
    }
    return false;
}

class SketchState : public itg::ofxState<ofxEasyFft> {
public:
    class CircleData {
    public:
        int index;
        bool invert;
    };
    
    enum Mode {
        Cats,
        Dogs,
        Smiles
    };
    
    string getName() {
        return "Sketches";
    }
    
    void setup() {
        _maxSamples = 10000;

        loadDrawings("full-simplified-smiley face.ndjson", _smiles);
        loadDrawings("full-simplified-dog.ndjson", _dogs);
        loadDrawings("full-simplified-cat.ndjson", _cats);
        
        setupTiles();
        setupPhysics();
        
        _invert = false;
        _mode = Cats;
        
        _post.init(ofGetWidth(), ofGetHeight());
        _post.createPass<FxaaPass>()->setEnabled(true);
        _post.createPass<BloomPass>()->setEnabled(true);
    }
    
    void update() {
        vector<float> audio = getSharedData().getAudio();
        float curVol = Util::calcVolume(audio);
        _smoothedVol *= SMOOTH_FACTOR;
        _smoothedVol += (1.0 - SMOOTH_FACTOR) * curVol;
        _scaledVol = ofMap(_smoothedVol, 0.0, VOLUME_MAX, 0.0, 1.0, true);
        
        if (_mode == Cats || _mode == Dogs) {
            updateTiles();
        } else if (_mode == Smiles) {
            updatePhysics();
        }
    }
    
    void draw(){
        if (_mode == Cats || _mode == Dogs) {
            drawTiles();
        } else if (_mode == Smiles) {
            drawPhysics();
        }
        
        if (_debugMode) {
            ofSetColor(128);
            ofDrawRectangle(0, 0, 200, 50);
            ofSetColor(255);
            ofDrawRectangle(0, 0, 200 * _scaledVol, 50);
        }
    }
    
    void keyPressed(int key) {
        if (key == OF_KEY_RIGHT) {
            if (_mode == Cats) {
                _mode = Dogs;
                _invert = true;
            } else if (_mode == Dogs) {
                _mode = Smiles;
                _invert = false;
            } else if (_mode == Smiles) {
                _mode = Cats;
                _invert = false;
            }
        } else if (key == 'd') {
            _debugMode = !_debugMode;
        }
    }
private:
    void setupTiles() {
        _nCols = 11;
        _nRows = 8;
        
        for (int i = 0; i < _nCols * _nRows; i++) {
            _indices.push_back(i);
            ofxTween tween;
            tween.setParameters(_linear, ofxTween::easeInOut, 0.0, 0.0, 0, 0);
            _tweens.push_back(tween);
        }
        
        _lastUpdate = static_cast<int>(ofGetElapsedTimef());
    }
    
    void updateTiles() {
        int t = static_cast<int>(ofGetElapsedTimeMillis() / 30);
        if (t != _lastUpdate) {
            vector<int> list;
            int n = ofMap(_scaledVol, 0.25, 0.75, 0, 3, true);
            if (n != 0) {
                pickIndices(list, n);
                for (int i = 0; i < list.size(); i++) {
                    _indices[list[i]] = ofRandom(_maxSamples);
                    _tweens[list[i]].setParameters(_linear, ofxTween::easeInOut, 1.0, 0.0, 500, 0);
                }
            }
            _lastUpdate = t;
        }
        
        for (int i = 0; i < _nRows * _nCols; i++) {
            _tweens[i].update();
        }
    }
    
    void drawTiles() {
        ofPushMatrix();
        ofRotate(15.0);
        ofTranslate(0, -300);
        
        _invert ? ofBackground(255) : ofBackground(0);
        for (int j = 0; j < _nRows; j++) {
            for (int i = 0; i < _nCols; i++) {
                int index = i + j * _nCols;
                float v = _tweens[index].getTarget(0) * 255;
                ofSetColor(_invert ? 255 - v : v);
                ofDrawRectangle(128 * i, 128 * j + 16, 128, 128);
                
                float x = 128 * (i + 0.25);
                float y = 128 * (j + 0.25) + 16;
                ofPushMatrix();
                ofTranslate(x, y);
                ofScale(0.25, 0.25);
                
                if (_mode == Cats) {
                    _cats[_indices[index]].setStrokeColor(ofColor(_invert ? v : 255 - v));
                    _cats[_indices[index]].draw(0, 0);
                } else {
                    _dogs[_indices[index]].setStrokeColor(ofColor(_invert ? v : 255 - v));
                    _dogs[_indices[index]].draw(0, 0);
                }
                ofPopMatrix();
            }
        }
        ofPopMatrix();
    }
    
    void setupPhysics() {
        _box2d.init();
        _box2d.enableEvents();
        _box2d.setGravity(0, 10);
        _box2d.createGround();
        _box2d.setFPS(60.0);
        _box2d.registerGrabbing();
        
        ofAddListener(_box2d.contactStartEvents, this, &SketchState::onContactStart);
        ofAddListener(_box2d.contactEndEvents, this, &SketchState::onContactEnd);
    }
    
    void updatePhysics() {
        ofRemove(_circles, removeShapeOffScreen);
        _box2d.update();
        
        float prob = ofMap(_scaledVol, 0.25, 0.75, 0.0, 1.0, true);
        
        if (ofRandom(1.0) < 0.05) {
            float x = ofRandom(0, ofGetWidth());
            addCircle(x, -50, ofRandom(20, 50));
        }
        
        ofxEasyFft& fft = getSharedData();
        vector<float> buffer = fft.getBins();
        Util::normalize(buffer);
        
        _ground.clear();
        _groundLine.clear();
        _groundLine.addVertex(0, ofGetHeight());
        int n = MIN(256, buffer.size());
        for (int i = 0; i < n; i+=8) {
            float x1 = ofMap(i, 0, n, 0, ofGetWidth(), true);
            float x2 = ofMap(i + 8, 0, n, 0, ofGetWidth(), true);
            float y = ofMap(buffer[i], 0, 1, ofGetHeight(), ofGetHeight() * 0.5, true);
            _groundLine.addVertex(x1, y);
            _groundLine.addVertex(x2, y);
        }
        _groundLine.addVertex(ofGetWidth(), ofGetHeight());
        _ground.addVertexes(_groundLine);
        _ground.create(_box2d.getWorld());
    }
    
    void drawPhysics() {
        _post.begin();
        ofBackground(0);
        
        for (int i = 0; i < _circles.size(); i++) {
            ofPoint p = _circles[i]->getPosition();
            float r = _circles[i]->getRadius();
            float rad = _circles[i]->getRotation();
            
            CircleData * data = (CircleData*)_circles[i].get()->getData();
            bool invert = data->invert;
            int index = data->index;
            float scale = r / 128.0;
            
            ofPushMatrix();
            ofTranslate(p);
            ofRotate(rad);
            //ofSetColor(invert ? 0 : 255);
            //ofDrawCircle(0, 0, r);
            ofScale(scale, scale);
            _smiles[index].setStrokeColor(ofColor(invert ? 255 : 0));
            _smiles[index].setStrokeWidth(3.0f);
            _smiles[index].draw(-128, -128);
            ofPopMatrix();
            
            ofSetColor(255);
            _tessellator.tessellateToMesh(_groundLine, OF_POLY_WINDING_ODD, _mesh);
            _mesh.draw();
        }
        _post.end();
    }
    
    void addCircle(float x, float y, float r) {
        shared_ptr<ofxBox2dCircle> c = shared_ptr<ofxBox2dCircle>(new ofxBox2dCircle);
        c.get()->setPhysics(1.0, 0.7, 0.9);
        c.get()->setup(_box2d.getWorld(), x, y, r);
        
        c.get()->setData(new CircleData());
        CircleData* data = (CircleData*)c.get()->getData();
        data->index = static_cast<int>(ofRandom(_maxSamples));
        data->invert = true;
        
        _circles.push_back(c);
    }
    
    void onContactStart(ofxBox2dContactArgs &e) {
        if(e.a != NULL && e.b != NULL) {
            
            // if we collide with the ground we do not
            // want to play a sound. this is how you do that
            if (e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle) {
                
                CircleData * aData = (CircleData*)e.a->GetBody()->GetUserData();
                CircleData * bData = (CircleData*)e.b->GetBody()->GetUserData();
                
                if (aData) {
                    aData->index = static_cast<int>(ofRandom(_maxSamples));
                }
                
                if (bData) {
                    bData->index = static_cast<int>(ofRandom(_maxSamples));
                }
            }
        }
    }
    
    void onContactEnd(ofxBox2dContactArgs &e) {
    }
    
    void loadDrawings(string filename, vector<ofPath>& container) {
        ofFile file(filename);
        ofBuffer buffer(file);
        int n = 0;
        container.clear();
        for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
            string line = *it;
            ofxJSONElement json;
            bool ret = json.parse(line);
            if (ret) {
                auto strokes = json["drawing"];
                
                ofPath path;
                for (int i = 0; i < strokes.size(); i++) {
                    auto stroke = strokes[i];
                    auto xVerts = strokes[i][0];
                    auto yVerts = strokes[i][1];
                    for (int j = 0; j < xVerts.size(); j++) {
                        float x = xVerts[j].asInt();
                        float y = yVerts[j].asInt();
                        if (j == 0) {
                            path.moveTo(x, y);
                        } else {
                            path.lineTo(x, y);
                        }
                    }
                }
                path.setFilled(false);
                path.setStrokeWidth(1.0f);
                path.setStrokeColor(ofColor::white);
                
                container.push_back(path);
                n++;
                if (n == _maxSamples) {
                    break;
                }
            }
        }
        buffer.clear();
    }
    
    void pickIndices(vector<int>& list, int n) {
        list.clear();
        for (int i = 0; i < _nRows * _nCols; i++) {
            list.push_back(i);
        }
        
        while (true) {
            int k = list.size();
            int index = static_cast<int>(ofRandom(k));
            list.erase(list.begin() + index);
            if (list.size() == n) {
                break;
            }
        }
    }
        
    vector<ofPath> _cats, _dogs, _smiles;
    int _nCols, _nRows, _maxSamples;
    vector<int> _indices;
    
    vector<ofxTween> _tweens;
    ofxEasingLinear _linear;
    
    int _lastUpdate;
    bool _invert;
    bool _debugMode = false;
    
    Mode _mode;
    
    float _smoothedVol = 0, _scaledVol = 0;
    
    ofxBox2d _box2d;
    vector<shared_ptr<ofxBox2dCircle> > _circles;
    ofxBox2dEdge _ground;
    ofPolyline _groundLine;
    ofTessellator _tessellator;
    ofMesh _mesh;
    
    ofxPostProcessing _post;
};
