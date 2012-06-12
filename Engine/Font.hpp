#ifndef Visualisierung2_FontEngine_hpp
#define Visualisierung2_FontEngine_hpp

#include <d3d11.h>
#include <D3DX10math.h>
#include <fstream>

#include "textureclass.h"

class Font
{

private:

	struct FontType {
		float left;
		float right;
		int size;
	};

	struct VertexType {
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

public:

	Font();
	Font(const Font &);
	~Font();

	bool Initialize(ID3D11Device *, char *, WCHAR *);
	void Shutdown();

	ID3D11ShaderResourceView *getTexture();

	void buildVertexArray(void *, char *, float, float);

private:

	bool loadFontData(char *);
	void releaseFontData();
	bool loadTexture(ID3D11Device *, WCHAR *);
	void releaseTexture();

	FontType *m_Font;
	TextureClass *m_Texture;

};

#endif

#ifndef Visualisierung2_FontShader_hpp
#define Visualisierung2_FontShader_hpp

#include <D3DX11async.h>

class FontShader
{

private:

	struct ConstantBufferType {
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct PixelBufferType {
		D3DXVECTOR4 pixelColor;
	};

public:

	FontShader();
	FontShader(const FontShader &);
	~FontShader();

	bool Initialize(ID3D11Device *, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext *, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView *, D3DXVECTOR4);

private:

	bool initializeShader(ID3D11Device *, HWND, WCHAR *, WCHAR *);
	void shutdownShader();
	void outputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);
	bool setShaderParameters(ID3D11DeviceContext *, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView *, D3DXVECTOR4);
	void renderShader(ID3D11DeviceContext *, int);

	ID3D11VertexShader	*m_vertexShader;
	ID3D11PixelShader	*m_pixelShader;
	ID3D11InputLayout	*m_layout;
	ID3D11Buffer		*m_constantBuffer;
	ID3D11SamplerState	*m_sampleState;

	ID3D11Buffer		*m_pixelBuffer;

};

#endif