#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

#define SHADOWSIZE 4096

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");

	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");

	lightShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");

	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");

	cubeShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !shadowShader->LoadSuccess() || !cubeShader->LoadSuccess()) {
		return;
	}
	//Shadow buffer code
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	quad = Mesh::GenerateQuad();

	test = Mesh::LoadFromMeshFile("Cube.msh");

	fish = Mesh::LoadFromMeshFile("bass2.msh");
	fishAnim = new MeshAnimation("bass2.anm");
	fishMat = new MeshMaterial("bass2.mat");


	heightMap = new HeightMap(TEXTUREDIR"noise2.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	squareTex = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	squareBump = SOIL_load_OGL_texture(TEXTUREDIR"brickDOT3.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg",
		TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !squareTex) {
		return;
	}

	setTextureRepeating(earthTex, true);
	setTextureRepeating(earthBump, true);
	setTextureRepeating(waterTex, true);
	setTextureRepeating(squareTex, true);
	

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));

	light = new Light(heightmapSize * Vector3(1.0f, 3.5f, 0.0f), Vector4(1, 1, 1, 1), 2.0f * heightmapSize.x);

	projMatrix = Matrix4::Perspective(1.0f, 25000.0f, (float)width / (float)height, 45.0f);

	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	waterBob = 0.0f;
	init = true;
}
Renderer::~Renderer(void) {
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	delete camera;
	delete heightMap;
	delete quad;
	delete test;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete shadowShader;
	delete light;

}
void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	//viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.2f;
	waterCycle += dt * 0.05f;
	waterBob = sin(dt);

}
void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	

	DrawSkybox();
	DrawHeightmap();
	DrawCube();
	DrawWater();
	DrawShadowScene();
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 25000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0.2f, 0.1f, 0.2f));

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 164.0f);
	shadowMatrix = projMatrix * viewMatrix;
	
	//modelMatrix.ToIdentity()
	modelMatrix = Matrix4::Translation((heightMap->GetHeightmapSize()) * Vector3(0.4f, 2.0f, 0.2f)) * Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f));
	UpdateShaderMatrices();
	
	test->Draw();

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	heightMap->Draw();
	

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}
void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*light);
	//glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "shadowTex"), 2);

	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	heightMap->Draw();



}

void Renderer::DrawCube() {
	BindShader(lightShader);

	SetShaderLight(*light);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, squareTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, squareBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glActiveTexture(GL_TEXTURE2);

	glBindTexture(GL_TEXTURE_2D, shadowTex);
	

	modelMatrix = Matrix4::Translation((heightMap->GetHeightmapSize()) * Vector3(0.4f, 2.0f, 0.2f)) * Matrix4::Scale(Vector3(100.0f,100.0f,100.0f));
	UpdateShaderMatrices();
	test->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix = (Matrix4::Translation(hSize * 0.5f) /* Matrix4::Translation(Vector3(1.0f, 50.0f*waterBob,1.0f))*/) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10))
		* Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();

	quad->Draw();
}