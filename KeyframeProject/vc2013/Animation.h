#pragma once


enum ExtrapolationMode{
	constant,
	linear,
	cyclic,
	cyclic_offset,
	bounce
};

enum TangentType{
	flat,
	Linear,
	smooth,
	value
};

class Keyframe
{
public:
	Keyframe();
	Keyframe(float Time, float Value, float TangentIn, float TangentOut, char RuleIn, char RuleOut);

	float Time;
	float Value;
	float TangentIn, TangentOut;
	TangentType RuleIn, RuleOut;
	float A, B, C, D;	//cubic coefficients
	float spanScale;	//scale of time beween this keyframe and the next (nextKeyframe.Time - nextKeyframe.Time)
};

class KeyframeChannel
{
public:
	KeyframeChannel();
	std::vector<Keyframe> keyframes;
	ExtrapolationMode minmode;
	ExtrapolationMode maxmode;
	void Precompute();
	float EvaluateCubic(float time, int i);
	void computeCubicCoefficients(int i);
	void computeTangentIn(int i);
	void computeTangentOut(int i);
	float InvLerpTime(float t, int i);
	float MapTime(float t);
	float Evaluate(float time);
	float extrapolate(int flag, float time);
	std::vector<int> findSpan(float time);

	ci::mat4 hermiteBasis;
};

class full_channel{
public:
	full_channel();
	full_channel(KeyframeChannel channelx, KeyframeChannel channely, KeyframeChannel channelz);
	std::vector<KeyframeChannel> channels;
};

/*TODO: 
	-implement actual animation functions
	-?patch up init function to load channels into full_channels 
		-full_channels hold the x, y, z channel for each respective joint
	*/
class Animation{
public:
	Animation();
	void init(const std::string& filename);
	//std::vector<KeyframeChannel> channels;
	std::vector<full_channel> full_channels;
	ExtrapolationMode Animation::getMode(std::string mode);

	TangentType getType(std::string type);
	ci::vec2 range;
	std::ifstream file;
	//~Animation();
};