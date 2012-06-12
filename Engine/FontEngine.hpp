#ifndef Visualisierung2_TextRenderer_hpp
#define Visualisierung2_TextRenderer_hpp

#include "Font.hpp"

class FontEngine
{

private:

	struct SentenceType {
		ID3D11Buffer *vertexBuffer;
		ID3D11Buffer *indexBuffer;
		int vertexCount;
		int indexCount;
		int maxLength;
		float red;
		float green;
		float blue;
	};

	struct VertexType {
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

public:

	FontEngine();
	FontEngine(const FontEngine &);
	~FontEngine();

	bool Initialize(ID3D11Device *device, ID3D11DeviceContext *deviceContext, HWND hwnd, int, int, D3DXMATRIX worldMatrix);
	void Shutdown();
	bool Render(ID3D11DeviceContext *deviceContext, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix);
	
	bool updateSentence(SentenceType *, char *, int, int, float red, float green, float blue, ID3D11DeviceContext *deviceContext);

	SentenceType	*m_sentence1;
	SentenceType	*m_sentence2;

private:

	bool initializeSentence(SentenceType **, int, ID3D11Device *device);
	void releaseSentence(SentenceType **);
	bool renderSentence(ID3D11DeviceContext *deviceContext, SentenceType *sentence, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix);

	Font			*m_Font;
	FontShader		*m_FontShader;
	int				m_screenWidth;
	int				m_screenHeight;
	D3DXMATRIX		m_baseViewMatrix;

};

#endif