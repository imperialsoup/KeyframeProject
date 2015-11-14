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

Keyframe::Keyframe(){}

KeyframeChannel::KeyframeChannel(){

}

/*loops through keyframes to precompute TangentIn, TangentOut, Cubic Coefficients, and spanScale for each keyframe in the channel*/
void KeyframeChannel::Precompute(){
	//compute in/out tangent values for each keyframe
	for (int i = 0; i < keyframes.size() - 1; i++){
		computeTangentIn(i);
		computeTangentOut(i);
		computeCubicCoefficients(i);
		//precompute spanScale
		keyframes[i].spanScale = 1 / (keyframes[i + 1].Time - keyframes[i].Time);
	}
}

/*compute and sets cubic coefficients A, B, C, D, for keyframe[i]*/
void KeyframeChannel::computeCubicCoefficients(int i){
	//t1 - t0
	float scale = keyframes[i + 1].Time - keyframes[i].Time; 
	//[p0, p1, v0, v1]  --> [p0, p1, (t1-t0)v0, (t1-t0)v1)]
	vec4 span = vec4(keyframes[i].Value, keyframes[i + 1].Value, scale * keyframes[i].TangentOut, scale * keyframes[i + 1].TangentIn);	
	//[a, b, c, d] = [p0, p1, v0, v1] * inv(hermiteBasis)
	vec4 abcd = span * hermiteBasis;		
	//store the coefficients in the first keyframe of the span
	keyframes[i].A = abcd[0];
	keyframes[i].B = abcd[1];
	keyframes[i].C = abcd[2];
	keyframes[i].D = abcd[3];
}

/*computes and sets the tangentIn for keyframe[i]*/
//TODO: Find a way to combine computeTangentIn/computeTangentOut, and fix case: value
void KeyframeChannel::computeTangentIn(int i){
	switch (keyframes[i].RuleIn){
	case flat:	keyframes[i].TangentIn = 0;
		break;

	case Linear:keyframes[i].TangentIn = ((keyframes[i].Value - keyframes[i - 1].Value) / (keyframes[i].Time - keyframes[i - 1].Time));
		break;

	case smooth:if (i == 0 || i == keyframes.size() - 1){//TODO: find a way to redirect this neatly to Linear case for first/last keyframe
		keyframes[i].TangentIn = ((keyframes[i].Value - keyframes[i - 1].Value) / (keyframes[i].Time - keyframes[i - 1].Time));
	}
				else {
					keyframes[i].TangentIn = ((keyframes[i + 1].Value - keyframes[i - 1].Value) / (keyframes[i + 1].Time - keyframes[i - 1].Time));
				}
				break;
	case value: //can only be specified on outgoing tangents..hmm...
		//tangent will be a direct change from the previous value to this one...
		break;
	}
}

/*computes and sets the tangentOut for keyframe[i]*/
void KeyframeChannel::computeTangentOut(int i){

	switch (keyframes[i].RuleOut){
	case flat:	keyframes[i].TangentOut = 0;
		break;
	case Linear:keyframes[i].TangentOut = ((keyframes[i].Value - keyframes[i + 1].Value) / (keyframes[i].Time - keyframes[i + 1].Time));
		break;
	case smooth:
		if (i == 0 || i == keyframes.size() - 1){//if first or last, use the linear rule (TODO: find a way to redirect this neatly to Linear case 
			keyframes[i].TangentOut = ((keyframes[i].Value - keyframes[i + 1].Value) / (keyframes[i].Time - keyframes[i + 1].Time));
		}
		else {
			keyframes[i].TangentOut = keyframes[i].TangentIn; //TangentIn should already be computed
		}
		break;
	case value: keyframes[i].TangentOut = keyframes[i].Value;
		keyframes[i].A = keyframes[i].B = keyframes[i].C = keyframes[i].D = 0;
		break;
	}

}

/*returns the value of the channel for a given time*/
float KeyframeChannel::Evaluate(float time){
	float value = 0;

	//find the span 
	vector<int> span_status = findSpan(time);

	switch (span_status[0]){
					//t falls before first or after last key : extrapolate
		case -1 :	if (span_status[1] == 0)	
						value = extrapolate(0);
					else	
						value = extrapolate(1);
					break;
		case 0 :	//t falls exactly on some key : return key value
					value = keyframes[span_status[1]].Value;
					break;
		case 1 :	//t falls b/w 2 keys : evaluate cubic equation
					value = EvaluateCubic(time, span_status[1]);
					break;
	}	
	return value;
}

/*computes and returns the value of the channel for time t using span after keyframes[i]*/
float KeyframeChannel::EvaluateCubic(float time, int i){
	float u = InvLerpTime(time, i);
	float x = pow(keyframes[i].A * u, 3) + pow(keyframes[i].B* u, 2) + keyframes[i].C * u + keyframes[i].D;
	return x;
}

/*computes and returns an interpolated value (0..1) for time t using span after keyframes[i]*/
float KeyframeChannel::InvLerpTime(float t, int i){
	float u = t - keyframes[i].Time * keyframes[i].spanScale;
	return u;
}

/*extrapolate(): returns a value based on extrapolation rules*/
//TODO: finish extrapolation evaluations
float KeyframeChannel::extrapolate(int flag){
	ExtrapolationMode mode;
	Keyframe frame;
	float value;

	if (flag == 0) {	//before 1st keyframe, use 1st keyframe & minmode
		mode = minmode;
		frame = keyframes[0];
		}
	else{				//after last keyframe, use last keyframe & maxmode
		mode = maxmode;
		frame = keyframes[keyframes.size() - 1];
		}

	switch (mode){
		case constant:	value = frame.Value;
						break;
		case linear:	value = frame.TangentIn; //TODO: replace with evaluation function
						break;
		case cyclic:	//repeat channel
						break;
		case cyclic_offset:	//repeat with value offset
						break;
		case bounce :	//repeat alternating backwards forwards
						break;
	}

	return value;
}


/*findSpan():finds the span where time value falls and returns a vector containing span information*/
/*span_status[i]: Key 
*******************
span_status[0]: status value (see below key)
span_status[1]: index of relevant element 

span_status[0] = value	--> key:
********************************
span_status[0] = -1		-->	t falls before first or after last key
span_status[0] = 0		--> t falls exactly on some key 
span_status[0] = 1		--> t falls b/w 2 keys */
vector<int> KeyframeChannel::findSpan(float time){
	vector<int> status;

	//if in between 1st & last frame
	if (time > keyframes[0].Time && time < keyframes[keyframes.size() - 1].Time){
		for (int i = 0; i < keyframes.size() - 1; i++){
			// 1) time falls exactly on a keyframe
			if (time == keyframes[i].Time){
				status.push_back(0); status.push_back(i);
				break;
			}
			// 2) time falls between this keyframe and the next
			if (time < keyframes[i].Time && time < keyframes[i + 1].Time){
				status.push_back(1); status.push_back(i);
				break;
			}
		}
	}
	//3) time falls before 1st frame
	else if (time < keyframes[0].Time){
		status.push_back(-1); status.push_back(0);
	}
	//4) time falls after last frame
	else if (time > keyframes[keyframes.size() - 1].Time){
		status.push_back(-1); status.push_back(keyframes.size() - 1);
	}
	return status;
}