#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_BLACK);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(.5f, .5f, .5f));
	static float value = 0.0f;

	std::vector< std::vector <int > > ImageData = 
	{
		{2,8},
		{3,7},
		{2,3,4,5,6,7,8},
		{1,2,4,5,6,8,9},
		{0,1,2,3,4,5,6,7,8,9,10},
		{0,2,3,4,5,6,7,8,10},
		{0,2,8,10},
		{3,4,6,7}
	};

	for (int i = 0; i < ImageData.size(); ++i) 
	{
		for (int j = 0; j < ImageData[i].size(); ++j)
		{
			matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(ImageData[i][j] + value - 10.0f, ImageData.size() - i, 0.0f));
			//matrix4 m4Model = m4Translate * m4Scale;
			matrix4 m4Model = m4Scale * m4Translate;

			m_pMesh->Render(m4Projection, m4View, m4Model);
		}
	}
	value += 0.01f;
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}