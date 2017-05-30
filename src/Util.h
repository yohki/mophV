#pragma once

#include "ofMain.h"

#define SMOOTH_FACTOR 0.86
#define VOLUME_MAX 0.10

class Util {
public:
    static void normalize(vector<float>& data) {
        float maxValue = 0;
        for(int i = 0; i < data.size(); i++) {
            if(abs(data[i]) > maxValue) {
                maxValue = abs(data[i]);
            }
        }
        for(int i = 0; i < data.size(); i++) {
            data[i] /= maxValue;
        }
    }
    
    static float calcVolume(vector<float>& audio) {
        float curVol = 0.0;
        //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
        for (int i = 0; i < audio.size(); i++){
            float val = audio[i];
            curVol += val * val;
        }
        
        //this is how we get the mean of rms :)
        curVol /= (float)audio.size();
        
        // this is how we get the root of rms :)
        curVol = sqrt(curVol);
        return curVol;
    }
};