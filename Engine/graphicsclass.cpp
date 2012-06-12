////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_LightShader = 0;
	m_Light = 0;
	m_Text = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	D3DXMATRIX baseViewMatrix;

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);
	m_Camera->SetRotation(-30.0f, 35.0f, 0.0f);
	
	// Create the model object.
	m_Model = new ModelClass;
	if(!m_Model)
	{
		return false;
	}

	// Initialize the model object.
	result = m_Model->Initialize(m_D3D->GetDevice(), "../Engine/data/cube.txt", L"../Engine/data/seafloor.dds", hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create the light shader object.
	m_LightShader = new LightShaderClass;
	if(!m_LightShader)
	{
		return false;
	}

	// Initialize the light shader object.
	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the light object.
	m_Light = new LightClass;
	if(!m_Light)
	{
		return false;
	}

	// Initialize the light object.
	m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);
	m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetSpecularPower(32.0f);

	m_Text = new FontEngine();
	if (!m_Text) return false;
	result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenHeight, screenWidth, baseViewMatrix);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize Font Engine", L"Error", MB_OK);
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	if (m_Text) {
		m_Text->Shutdown();
		delete m_Text;
		m_Text = 0;
	}

	// Release the light object.
	if(m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the light shader object.
	if(m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	// Release the model object.
	if(m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}


bool GraphicsClass::Frame()
{
	bool result;
	static float rotation = 0.0f;


	// Update the rotation variable each frame.
	//rotation += (float)D3DX_PI * 0.005f;
	if(rotation > 360.0f)
	{
		rotation -= 360.0f;
	}
	
	// Render the graphics scene.
	result = Render(rotation);
	if(!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render(float rotation)
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	// Rotate the world matrix by the rotation value so that the triangle will spin.
	D3DXMatrixRotationY(&worldMatrix, rotation);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Model->Render(m_D3D->GetDeviceContext());

	// Render the model using the light shader.
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
								   m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), 
								   m_Camera->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
	if(!result) return false;

	m_D3D->TurnOffZBuffer();
	m_D3D->TurnOnAlphaBlending();

	D3DXVECTOR3 camPosition = m_Camera->GetPosition();
	D3DXVECTOR3 camRotation = m_Camera->GetRotation();

	char sentence1[128], sentence2[128];
	
	sprintf(sentence1, "Position: %.2f, %.2f, %.2f", camPosition.x, camPosition.y, camPosition.z);
	sprintf(sentence2, "Rotation: %.2f, %.2f, %.2f", camRotation.x, camRotation.y, camRotation.z);
	
	m_Text->updateSentence(m_Text->m_sentence1, sentence1, -90, 110, 1.0, 1.0, 1.0, m_D3D->GetDeviceContext());
	m_Text->updateSentence(m_Text->m_sentence2, sentence2, -90, 130, 1.0, 1.0, 1.0, m_D3D->GetDeviceContext());

	result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result) return false;

	m_D3D->TurnOffAlphaBlending();
	m_D3D->TurnOnZBuffer();

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}

void GraphicsClass::move_camera(float x,float z)
{
	D3DXVECTOR3 Position = m_Camera->GetPosition();
	D3DXVECTOR3 DefaultForward = D3DXVECTOR3(0.0f,0.0f,1.0f);
	D3DXVECTOR3 DefaultRight = D3DXVECTOR3(1.0f,0.0f,0.0f);
	D3DXVECTOR3 Forward = DefaultForward;
	D3DXVECTOR3 Right = DefaultRight;

	D3DXMATRIX RotateYTempMatrix;
	D3DXMATRIX RotateXTempMatrix;
	D3DXMatrixRotationY(&RotateYTempMatrix, m_Camera->GetRotation().y * 0.0174532925f);
	D3DXMatrixRotationX(&RotateXTempMatrix, m_Camera->GetRotation().x * 0.0174532925f);

	D3DXVec3TransformNormal(&Right, &DefaultRight, &RotateYTempMatrix);
	D3DXVec3TransformNormal(&Forward, &DefaultForward, &RotateXTempMatrix);
	D3DXVec3TransformNormal(&Forward, &Forward, &RotateYTempMatrix);

	Position += x*Right;
	Position += z*Forward;

	m_Camera->SetPosition(Position);
	return;
}

void GraphicsClass::rotate_camera(float x,float y)
{
	D3DXVECTOR3 temp = m_Camera->GetRotation();
	m_Camera->SetRotation(temp.x+x,temp.y+y,temp.z);
}