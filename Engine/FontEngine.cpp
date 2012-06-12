#include "FontEngine.hpp"

FontEngine::FontEngine()
{
	this->m_Font = 0;
	this->m_FontShader = 0;

	this->m_sentence1 = 0;
	this->m_sentence2 = 0;
}

FontEngine::FontEngine(const FontEngine &)
{
}

FontEngine::~FontEngine()
{
}

bool FontEngine::Initialize(ID3D11Device *device, ID3D11DeviceContext *deviceContext, HWND hwnd, int screenWidth, int screenHeight, D3DXMATRIX baseViewMatrix)
{
	this->m_screenWidth = screenWidth;
	this->m_screenHeight = screenHeight;
	this->m_baseViewMatrix = baseViewMatrix;

	this->m_Font = new Font();
	if (!this->m_Font) return false;

	bool result = this->m_Font->Initialize(device, "../Engine/data/fontdata.txt", L"../Engine/data/font.dds");
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the font object", L"Error", MB_OK);
		return false;
	}

	this->m_FontShader = new FontShader();
	if (!this->m_FontShader) return false;

	result = this->m_FontShader->Initialize(device, hwnd);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the font shader object", L"Error", MB_OK);
		return false;
	}

	result = this->initializeSentence(&this->m_sentence1, 128, device);
	if (!result) return false;

	result = this->updateSentence(this->m_sentence1, "Hello World", 100, 100, 1.0f, 1.0f, 1.0f, deviceContext);
	if (!result) return false;

	result = this->initializeSentence(&this->m_sentence2, 128, device);
	if (!result) return false;

	result = this->updateSentence(this->m_sentence2, "This is a test", 100, 200, 1.0f, 1.0f, 1.0f, deviceContext);
	if (!result) return false;

	return true;
}

void FontEngine::Shutdown()
{
	this->releaseSentence(&this->m_sentence1);
	this->releaseSentence(&this->m_sentence2);
	
	if (this->m_FontShader) {
		this->m_FontShader->Shutdown();
		delete this->m_FontShader;
		this->m_FontShader = 0;
	}

	if (this->m_Font) {
		this->m_Font->Shutdown();
		delete this->m_Font;
		this->m_Font = 0;
	}
}

bool FontEngine::Render(ID3D11DeviceContext *deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
	bool result = this->renderSentence(deviceContext, this->m_sentence1, worldMatrix, orthoMatrix);
	if (!result) return false;

	result = this->renderSentence(deviceContext, this->m_sentence2, worldMatrix, orthoMatrix);
	if (!result) return false;

	return true;
}

bool FontEngine::initializeSentence(SentenceType **sentence, int maxLength, ID3D11Device *device)
{
	*sentence = new SentenceType();
	if (!*sentence) return false;

	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;
	(*sentence)->maxLength = maxLength;
	(*sentence)->vertexCount = 6 * maxLength;
	(*sentence)->indexCount = (*sentence)->vertexCount;

	VertexType *vertices = new VertexType[(*sentence)->vertexCount];
	if (!vertices) return false;

	unsigned long *indices = new unsigned long[(*sentence)->indexCount];
	if (!indices) return false;

	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	for (int i = 0; i < (*sentence)->indexCount; i++) {
		indices[i] = i;
	}

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if (FAILED(result)) return false;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if (FAILED(result)) return false;

	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

bool FontEngine::updateSentence(SentenceType *sentence, char *text, int posX, int posY, float red, float green, float blue, ID3D11DeviceContext *deviceContext)
{
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	int numLetters = (int)strlen(text);
	if (numLetters > sentence->maxLength) return false;

	VertexType *vertices = new VertexType[sentence->vertexCount];
	if (!vertices) return false;

	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	float drawX = (float)(((this->m_screenWidth / 2) * -1) + posX);
	float drawY = (float)((this->m_screenHeight / 2) - posY);

	this->m_Font->buildVertexArray((void *)vertices, text, drawX, drawY);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	VertexType *verticesPtr = (VertexType *)mappedResource.pData;
	memcpy(verticesPtr, (void *)vertices, (sizeof(VertexType) * sentence->vertexCount));

	deviceContext->Unmap(sentence->vertexBuffer, 0);

	delete [] vertices;
	vertices = 0;

	return true;
}

void FontEngine::releaseSentence(SentenceType **sentence)
{
	if (*sentence) {
		if ((*sentence)->vertexBuffer) {
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}
		if ((*sentence)->indexBuffer) {
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}

		delete *sentence;
		*sentence = 0;
	}
}

bool FontEngine::renderSentence(ID3D11DeviceContext *deviceContext, SentenceType *sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXVECTOR4 pixelColor = D3DXVECTOR4(sentence->red, sentence->green, sentence->blue, 1.0f);
	
	bool result = this->m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, this->m_baseViewMatrix, orthoMatrix, this->m_Font->getTexture(), pixelColor);
	if (!result) return false;

	return true;
}