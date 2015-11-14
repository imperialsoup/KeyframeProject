#ifndef KeyframeProject_Skin_h
#define KeyframeProject_Skin_h

//skin vertice class
class SkinVertex{
public:
	//SkinVertex();

	ci::vec4 position;				//position
	glm::mat4x4 M;					//transformation matrix
	std::map<int, float> weights;
	ci::vec4 normal;				//normal

	ci::vec4 trans_position;
	ci::vec4 trans_normal;

	inline void computeVertex();
	void computeNormal();
	void computeTransMatrix();
	void update();
};

class Skin{
public:
	Skin();
	void init(const std::string& filename, Skeleton * skeleton);
	std::vector<ci::vec4>initVertices(int numVertices);
	std::vector<std::map<int, float>> initSkinWeight(int numVertices);
	void initTriangles(int numTriangles);
	void initBindings(int numBindings);
	std::vector<ci::vec4> initNormals(int numVertices);
	void draw();
	void update();

	std::vector<SkinVertex> skinvertexes;
	std::vector<glm::mat4x4> skinMatrices;
	std::vector<ci::vec3> triangles;
	std::vector<glm::mat4x4> bindings;

	std::ifstream file;
	Skeleton * skeleton;	//the skeleton this skin is associated with
};
#endif
