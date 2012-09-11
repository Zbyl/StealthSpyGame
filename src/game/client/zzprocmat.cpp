//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
// identifier was truncated to '255' characters in the debug information
#pragma warning(disable: 4786)

#include "ProxyEntity.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/ITexture.h"
#include "bitmap/TGALoader.h"
#include "view.h"
#include "datacache/idatacache.h"
#include "materialsystem/IMaterial.h"
#include "vtf/vtf.h"
#include "pixelwriter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CZZMaterialProxy;

class CZZTextureRegen : public ITextureRegenerator
{
public:
	CZZTextureRegen() {} 
	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	virtual void Release() {}
};

class CZZMaterialProxy : public CEntityMaterialProxy
{
public:
	CZZMaterialProxy();
	virtual ~CZZMaterialProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind(C_BaseEntity *pC_BaseEntity );
	virtual void Release();
	virtual IMaterial *GetMaterial();

private:
	IMaterial *m_pMaterial;
	IMaterialVar *m_pTextureVar;
	ITexture *m_pTexture;
	CZZTextureRegen m_TextureRegen;
};


void CZZTextureRegen::RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
{
	//m_pProxy->GenerateCamoTexture( pTexture, pVTFTexture );

	CPixelWriter pixelWriter;
	pixelWriter.SetPixelMemory( pVTFTexture->Format(), 
		pVTFTexture->ImageData( 0, 0, 0 ), pVTFTexture->RowSizeInBytes( 0 ) );

	// Now upload the part we've been asked for
	int xmax = pSubRect->x + pSubRect->width;
	int ymax = pSubRect->y + pSubRect->height;
	int x, y;

	for( y = pSubRect->y; y < ymax; ++y )
	{
		pixelWriter.Seek( pSubRect->x, y );

		for( x=pSubRect->x; x < xmax; ++x )
		{
			pixelWriter.WritePixel( y%256, 0, 0, 255 );
		}
	}
}

CZZMaterialProxy::CZZMaterialProxy()
{
	m_pMaterial = NULL;
	m_pTextureVar = NULL;
	m_pTexture = NULL;
}

CZZMaterialProxy::~CZZMaterialProxy()
{
}

void CZZMaterialProxy::Release()
{
	// Disconnect the texture regenerator...
	if (m_pTexture)
	{
		//ITexture *pTexture = m_pTextureVar->GetTextureValue(); <- powoduje access violation
		if (m_pTexture)
			m_pTexture->SetTextureRegenerator(NULL);
	}
}

bool CZZMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	// ZZ return false; // hack!  Need to make sure that the TGA loader has a valid filesystem before trying
			// to load the camo pattern.

	m_pMaterial = pMaterial;

	// get pointers to material vars.
	bool found;
	m_pTextureVar = pMaterial->FindVar( "$baseTexture", &found );
	if( !found )
	{
		m_pTextureVar = NULL;
		return false;
	}

	m_pTexture = m_pTextureVar->GetTextureValue();
	if (m_pTexture)
		m_pTexture->SetTextureRegenerator( &m_TextureRegen );
	
	return true;
}

//-----------------------------------------------------------------------------
// Called when the texture is bound...
//-----------------------------------------------------------------------------
void CZZMaterialProxy::OnBind( C_BaseEntity *pEntity )
{
	if( !m_pTextureVar )
	{
		return;
	}
	
	if(!m_pTextureVar->IsTexture())  // Make sure it is a texture
		return;

	//ITexture *pTexture = m_pTextureVar->GetTextureValue();
	m_pTexture->Download();

	// Mark it so it doesn't get regenerated on task switch
	//m_pEnt = NULL;
}

IMaterial *CZZMaterialProxy::GetMaterial()
{
	return m_pMaterial;
}

EXPOSE_INTERFACE( CZZMaterialProxy, IMaterialProxy, "ZZProcMatRenderer" IMATERIAL_PROXY_INTERFACE_VERSION );
