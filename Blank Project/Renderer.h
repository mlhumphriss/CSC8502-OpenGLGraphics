#pragma once
#include "../nclgl/OGLRenderer.h"
class Camera;
class Shader;
class HeightMap;
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawShadowScene();
	void DrawCube();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* shadowShader;
	Shader* cubeShader;

	GLuint shadowTex;
	GLuint shadowFBO;

	HeightMap* heightMap;
	Mesh* quad;

	Light* light;
	Camera* camera;

	Mesh* test;

	Mesh* fish;
	MeshAnimation* fishAnim;
	MeshMaterial* fishMat;


	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint squareTex;
	GLuint squareBump;


	float waterRotate;
	float waterCycle;
	float waterBob;
};
