#include "AppClass.h"
void Application::InitVariables(void)
{
	//m_pWindow->setPosition(sf::Vector2i(710, 0));

	//Make MyMesh object
	m_MeshEnemy = new MyMesh();
	m_MeshEnemy->GenerateCube(1, C_PURPLE);
	m_MeshEnemy->GenerateCube(1, C_PURPLE);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	//ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	//m_pMesh->Render(m_pCameraMngr->GetProjectionMatrix(), m_pCameraMngr->GetViewMatrix(), ToMatrix4(m_qArcBall));
	//m_pMeshCircle->Render(m_pCameraMngr->GetProjectionMatrix(), m_pCameraMngr->GetViewMatrix(), ToMatrix4(m_qArcBall));
	//m_pCube->Render(m4Projection, m4View);
	
	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();

	static float value = 0.0f;
	matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(value, 2.0f, 3.0f));
	value += 0.01f;
	matrix4 m4Model = m4Translate;

	m_MeshEnemy->Render(m4Projection, m4View, m4Model);

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList("Skybox_02.png");
	
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
	if (m_pMeshCircle != nullptr)
	{
		delete m_pMeshCircle;
		m_pMeshCircle = nullptr;
	}
	SafeDelete(m_pMeshCone);
	//release GUI
	ShutdownGUI();
}