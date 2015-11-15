class Skeleton;

#ifndef KeyframeProject_Animation_h
#define KeyframeProject_Animation_h

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
	float MapTime(float t, bool bounceflag);
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

	inline void Precompute(){
		/*for each(KeyframeChannel channel in channels){
			channel.Precompute();
		}*/
		for (int j = 0; j < channels.size(); j++){
			channels[j].Precompute();
		}
	}

	inline ci::vec3 Evaluate(float time){
		ci::vec3 value;
		value.x = channels[0].Evaluate(time);
		value.y = channels[1].Evaluate(time);
		value.z = channels[2].Evaluate(time);
		return value;
	}
};

class Animation{
public:
	Animation();
	ci::Timer timer;
	bool start = false;
	void init(const std::string& filename, Skeleton * skeletonptr);
	Skeleton * skeleton;	//the skeleton this anim is associated with
	void animate();
	std::vector<full_channel> full_channels;
	ExtrapolationMode getMode(std::string mode);

	TangentType getType(std::string type);
	ci::vec2 range;
	std::ifstream file;
	//~Animation();
};

#endif