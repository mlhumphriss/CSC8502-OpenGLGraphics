#pragma once
#include "../nclgl/OGLRenderer.h"
class Camera;
class Shader;
class HeightMap;

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


	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint squareTex;
	GLuint squareBump;


	float waterRotate;
	float waterCycle;
};
