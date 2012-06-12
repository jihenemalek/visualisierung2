#include "Font.hpp"

Font::Font()
{
	m_Font = 0;
	m_Texture = 0;
}

Font::Font(const Font &other)
{
}

Font::~Font()
{
}

bool Font::Initialize(ID3D11Device *device, char *fontFilename, WCHAR *textureFilename)
{
	bool result;

	result = this->loadFontData(fontFilename);
	if (!result) return false;
	
	result = this->loadTexture(device, textureFilename);
	if (!result) return false;

	return true;
}

void Font::Shutdown()
{
	this->releaseTexture();
	this->releaseFontData();
}

bool Font::loadFontData(char *filename) 
{
	std::ifstream in;
	int i;
	char temp;

	this->m_Font = new FontType[95];
	if (!this->m_Font) return false;

	in.open(filename);
	if (in.fail()) return false;

	for (int i = 0; i < 95; i++) {
		in.get(temp);
		while(temp != ' ') {
			in.get(temp);
		}
		in.get(temp);
		while (temp != ' ') {
			in.get(temp);
		}

		in >> this->m_Font[i].left;
		in >> this->m_Font[i].right;
		in >> this->m_Font[i].size;
	}

	in.close();

	return true;
}

void Font::releaseFontData()
{
	if (this->m_Font) {
		delete [] this->m_Font;
		this->m_Font = 0;
	}
}

bool Font::loadTexture(ID3D11Device *device, WCHAR *filename)
{
	bool result;

	this->m_Texture = new TextureClass();
	if (!this->m_Texture) return false;

	result = this->m_Texture->Initialize(device, filename);
	if (!result) return false;

	return true;
}

void Font::releaseTexture()
{
	if (this->m_Texture) {
		this->m_Texture->Shutdown();
		delete this->m_Texture;
		this->m_Texture = 0;
	}
}

ID3D11ShaderResourceView * Font::getTexture()
{
	return this->m_Texture->GetTexture();
}

void Font::buildVertexArray(void *vertices, char *sentence, float drawX, float drawY)
{
	VertexType *vertexPtr;
	
	vertexPtr = (VertexType *)vertices;
	int numLetters = (int)strlen(sentence);
	int index = 0;

	for (int i = 0; i < numLetters; i++) {
		int letter = ((int)sentence[i]) - 32;

		if (letter == 0) {
			drawX = drawX + 3.0f;
		} else {
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(this->m_Font[letter].left, 0.0f);
			index++;
			vertexPtr[index].position = D3DXVECTOR3(drawX + this->m_Font[letter].size, drawY - 16, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(this->m_Font[letter].right, 1.0f);
			index++;
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY - 16, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(this->m_Font[letter].left, 1.0f);
			index++;

			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(this->m_Font[letter].left, 0.0f);
			index++;
			vertexPtr[index].position = D3DXVECTOR3(drawX + this->m_Font[letter].size, drawY, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(this->m_Font[letter].right, 0.0f);
			index++;
			vertexPtr[index].position = D3DXVECTOR3(drawX + this->m_Font[letter].size, drawY - 16, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(this->m_Font[letter].right, 1.0f);
			index++;

			drawX += this->m_Font[letter].size + 1.0f;
		}
	}
}

FontShader::FontShader()
{
	this->m_vertexShader = 0;
	this->m_pixelShader = 0;
	this->m_layout = 0;
	this->m_constantBuffer = 0;
	this->m_sampleState = 0;
	this->m_pixelBuffer = 0;
}

FontShader::FontShader(const FontShader &other)
{
}

FontShader::~FontShader()
{
}

bool FontShader::Initialize(ID3D11Device *device, HWND hwnd)
{
	bool result = this->initializeShader(device, hwnd, L"../Engine/font.vs", L"../Engine/font.ps");
	if (!result) return false;

	return true;
}

void FontShader::Shutdown()
{
	this->shutdownShader();
}

bool FontShader::Render(ID3D11DeviceContext *deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView *texture, D3DXVECTOR4 pixelColor)
{
	bool result = this->setShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);
	if (!result) return false;

	this->renderShader(deviceContext, indexCount);

	return true;
}

bool FontShader::initializeShader(ID3D11Device *device, HWND hwnd, WCHAR *vsFilename, WCHAR *psFilename)
{
	ID3D10Blob *errorMessage = 0;
	ID3D10Blob *vertexShaderBuffer = 0;
	ID3D10Blob *pixelShaderBuffer = 0;
	D3D11_BUFFER_DESC constantBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC pixelBufferDesc;
	
	HRESULT result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result)) {
		if (errorMessage) {
			this->outputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		} else {
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}
	
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result)) {
		if (errorMessage) {
			this->outputShaderErrorMessage(errorMessage, hwnd, psFilename);
		} else {
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &(this->m_vertexShader));
	if (FAILED(result)) return false;

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &(this->m_pixelShader));
	if (FAILED(result)) return false;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;
	
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &this->m_layout);
	if (FAILED(result)) return false;

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&constantBufferDesc, NULL, &this->m_constantBuffer);
	if (FAILED(result)) return false;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDesc, &this->m_sampleState);
	if (FAILED(result)) return false;

	pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
	pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelBufferDesc.MiscFlags = 0;
	pixelBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&pixelBufferDesc, NULL, &this->m_pixelBuffer);
	if (FAILED(result)) return false;

	return true;


}

void FontShader::shutdownShader()
{
	if (this->m_pixelBuffer) {
		this->m_pixelBuffer->Release();
		this->m_pixelBuffer = 0;
	}

	if (this->m_sampleState) {
		this->m_sampleState->Release();
		this->m_sampleState = 0;
	}

	if (this->m_constantBuffer) {
		this->m_constantBuffer->Release();
		this->m_constantBuffer = 0;
	}

	if (this->m_layout) {
		this->m_layout->Release();
		this->m_layout = 0;
	}

	if (this->m_pixelShader) {
		this->m_pixelShader->Release();
		this->m_pixelShader = 0;
	}

	if (this->m_vertexShader) {
		this->m_vertexShader->Release();
		this->m_vertexShader = 0;
	}
}

void FontShader::outputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, WCHAR *shaderFilename)
{
	char *compileErrors = (char *)(errorMessage->GetBufferPointer());
	unsigned long bufferSize = errorMessage->GetBufferSize();
	
	std::ofstream out;
	out.open("shader-error.txt");

	for (unsigned int i = 0; i < bufferSize; i++) {
		out << compileErrors[i];
	}

	out.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for details.", shaderFilename, MB_OK);
}

bool FontShader::setShaderParameters(ID3D11DeviceContext *deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView *texture, D3DXVECTOR4 pixelColor)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(this->m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	ConstantBufferType *dataPtr = (ConstantBufferType *)mappedResource.pData;
	
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(this->m_constantBuffer, 0);

	unsigned int bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &this->m_constantBuffer);
	deviceContext->PSSetShaderResources(0, 1, &texture);

	result = deviceContext->Map(this->m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	PixelBufferType *dataPtr2 = (PixelBufferType *)mappedResource.pData;
	dataPtr2->pixelColor = pixelColor;

	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &this->m_pixelBuffer);

	return true;
}

void FontShader::renderShader(ID3D11DeviceContext *deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(this->m_layout);
	
	deviceContext->VSSetShader(this->m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(this->m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &this->m_sampleState);
	
	deviceContext->DrawIndexed(indexCount, 0, 0);
}