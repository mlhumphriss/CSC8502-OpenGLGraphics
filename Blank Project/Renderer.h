#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/SceneNode.h"
class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;
class SceneNode;


class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawShadowScene();
	void DrawCube();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void RenderMeshMat();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* shadowShader;
	Shader* cubeShader;
	Shader* flatTexShader;
	Shader* skelShader;

	GLuint shadowTex;
	GLuint shadowFBO;
	GLuint texture;
	MeshMaterial* meshMaterial;


	HeightMap* heightMap;
	Mesh* quad;

	Light* light;
	Camera* camera;

	SceneNode* root;
	Frustum frameFrustum;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;

	Mesh* test;

	Mesh* fish;
	MeshAnimation* fishAnim;
	MeshMaterial* fishMat;
	vector<GLuint> matTextures;

	Mesh* boat;
	MeshMaterial* boatMat;


	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint squareTex;
	GLuint squareBump;
	GLuint boatTex;


	float waterRotate;
	float waterCycle;
	float waterBob;
	int currentFrame;
	float frameTime;
};
