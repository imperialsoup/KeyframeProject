#include "cinder/app/AppBase.h"
#include "cinder/cinder.h"
#include "cinder/Function.h"
#include "cinder/gl/gl.h"
#include <stdio.h>
#include "Animation.h"
#include <fstream>
#include "cinder/Json.h"

using namespace ci;
using namespace ci::app;
using namespace gl;
using namespace std;



Animation::Animation(){
}

/*loads and initalizes channels and keyframes from an .anim file*/
void Animation::init(const std::string& filename){

	ifstream file;
	console() << "Initializing Animation..." << std::endl;
	file.open(filename);

	if (!fs::exists(filename)){
		console() << "Animation.init() : Error, file not found." << std::endl;
		return;
	}
	if (file.is_open()){
		console() << "Animation.init() : File found, parsing now..." << std::endl;

		string input, trash, modein, modeout;
		file >> input; //"animation"
		int numchannels = 0;

		if (input == "animation"){
			file >> input;	// }
			file >> input;	//range
			file >> range.x; file >> range.y;
			file >> input; //numchannels
			file >> numchannels;

			Keyframe keyframe;
			KeyframeChannel ch = KeyframeChannel();
			Keyframe newKeyframe = Keyframe();
			//read in each full channel
			for (int i = 0; i < numchannels / 3; i++){
				full_channel fchannel = full_channel();
				//read in 3 channels 
				for (int j = 0; j < 3; j++){ //3 --> # of DOFs we are working with atm
					int numkeys = 0;

					file >> input; file >> trash;	//channel {
					file >> input; file >> modein; file >> modeout;	//extrapolate mode1 mode2
					ch.minmode = getMode(modein);
					ch.maxmode = getMode(modeout);
					ch.hermiteBasis = mat4(2, -2, 1, 1,
						-3, 3, -2, -1,
						0, 0, 1, 0,
						1, 0, 0, 0);

					file >> input; file >> numkeys; file >> trash;	//keys # {
					//make keys for new channel
					for (int i = 0; i < numkeys; i++){
						string tangentin, tangentout;

						file >> newKeyframe.Time; file >> newKeyframe.Value;
						file >> tangentin; file >> tangentout;//only takes 1 char

						//TODO: Fix so that a value tangent type reads in a float value for the tangent
						newKeyframe.RuleIn = getType(tangentin);
						newKeyframe.RuleOut = getType(tangentout);

						//add the key to new channel
						ch.keyframes.push_back(newKeyframe);
					}
					file >> trash; file >> trash;	//}end keyframe } end channel
					fchannel.channels.push_back(ch);	//have full channel push back new channel
					ch.keyframes.clear();
				}
				full_channels.push_back(fchannel);
				fchannel.channels.clear();
			}
		}
	}
	console() << "Animation.init() : Finished initializing animation. " << std::endl;
}

/*returns the TangentType associated with the given string*/
TangentType Animation::getType(string type){
	TangentType tangenttype = flat;
	if (type == "flat")
		tangenttype = flat;
	if (type == "linear")
		tangenttype = Linear;
	if (type == "smooth")
		tangenttype = smooth;
	if (type == "value") 
		tangenttype = value;
	return tangenttype;
}

/*returns the ExtrapolationMode associated with the given string*/
ExtrapolationMode Animation::getMode(string mode){
	ExtrapolationMode exmode = constant;
	if (mode == "constant")
		exmode = constant;
	if (mode == "linear")
		exmode = linear;
	if (mode == "cyclic")
		exmode = cyclic;
	if (mode == "cyclic_offset")
		exmode = cyclic_offset;
	if (mode == "bounce")
		exmode = bounce;
	return exmode;
}