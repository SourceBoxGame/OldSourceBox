//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dialog for selecting game configurations
//
//=====================================================================================//

#include <windows.h>

#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/ISystem.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ProgressBar.h>
#include "tier0/vcrmode.h"
#include "tier1/strtools.h"
#include "tier1/keyvaluesjson.h"
#include "bit7z.hpp"
#include "appframework/tier3app.h"

#include "RootPanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

#define DOWNLOAD_MAX_SIZE 1<<30 // 1 gig

RootPanel *g_pRootPanel = NULL;
extern UpdaterDownloadHandler* g_pUpdateDownloadHandler;
extern GameDownloadHandler* g_pGameDownloadHandler;
extern CHttpDownloader* g_pAPIDownloader;
extern CHttpDownloader* g_pFileDownloader;

class CModalPreserveMessageBox : public vgui::MessageBox
{
public:
	CModalPreserveMessageBox(const char *title, const char *text, vgui::Panel *parent)
		: vgui::MessageBox( title, text, parent )
	{
		m_PrevAppFocusPanel = vgui::input()->GetAppModalSurface();
	}

	~CModalPreserveMessageBox()
	{
		vgui::input()->SetAppModalSurface( m_PrevAppFocusPanel );
	}

public:
	vgui::VPANEL	m_PrevAppFocusPanel;
};
		
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
RootPanel::RootPanel( Panel *parent, const char *name ) : BaseClass( parent, name )
{
	g_pRootPanel = this;

	SetMinimizeButtonVisible( true );	

	SetSize( 192, 128 );
	SetSizeable( false );
	SetTitle( "Updater" , true );
	m_pUpdateButton = new Button(this, "Update", "Update SourceBox", this, "UpdateSourceBox");
	m_pUpdateButton->SetContentAlignment(Label::a_center);
	m_pUpdateProgress = new ProgressBar(this, "UpdateProgress");
	m_pUpdateProgress->SetSegmentInfo(2, 7);

}

//-----------------------------------------------------------------------------
// Destructor 
//-----------------------------------------------------------------------------
RootPanel::~RootPanel()
{
	g_pRootPanel = NULL;
}

void RootPanel::SetProgress(float p)
{
	m_pUpdateProgress->SetProgress(p);
}

void RootPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	SetSize(192, 128);
	m_pUpdateButton->SetBounds(16, 40, 192 - 32, 48);
	m_pUpdateProgress->SetBounds(16, 128 - 32, 192 - 32, 16);
}

//-----------------------------------------------------------------------------
// Purpose: Kills the whole app on close
//-----------------------------------------------------------------------------
void RootPanel::OnClose( void )
{
	BaseClass::OnClose();
	ivgui()->Stop();	
}

//-----------------------------------------------------------------------------
// Purpose: Parse commands coming in from the VGUI dialog
//-----------------------------------------------------------------------------
void RootPanel::OnCommand( const char *command )
{		
	if (strcmp(command, "UpdateSourceBox") == 0)
	{
		g_pAPIDownloader->BeginDownload("https://api.github.com/repos/SourceBoxGame/SourceBox/releases/latest");
		//g_pAPIDownloader->BeginDownload("http://192.168.1.30:27000/latest.json");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}


#include "curl/curl.h"
// curl callback functions

static size_t curlWriteFn(void* ptr, size_t size, size_t nmemb, void* stream)
{
	RequestContext_t* pRC = (RequestContext_t*)stream;
	if (pRC->nBytesTotal && pRC->nBytesCurrent + (size * nmemb) <= pRC->nBytesTotal)
	{
		Q_memcpy(pRC->data + pRC->nBytesCurrent, ptr, (size * nmemb));
		pRC->nBytesCurrent += size * nmemb;
	}
	return size * nmemb;
}


int Q_StrTrim(char* pStr)
{
	char* pSource = pStr;
	char* pDest = pStr;

	// skip white space at the beginning
	while (*pSource != 0 && isspace(*pSource))
	{
		pSource++;
	}

	// copy everything else
	char* pLastWhiteBlock = NULL;
	char* pStart = pDest;
	while (*pSource != 0)
	{
		*pDest = *pSource++;
		if (isspace(*pDest))
		{
			if (pLastWhiteBlock == NULL)
				pLastWhiteBlock = pDest;
		}
		else
		{
			pLastWhiteBlock = NULL;
		}
		pDest++;
	}
	*pDest = 0;

	// did we end in a whitespace block?
	if (pLastWhiteBlock != NULL)
	{
		// yep; shorten the string
		pDest = pLastWhiteBlock;
		*pLastWhiteBlock = 0;
	}

	return pDest - pStart;
}

static size_t curlHeaderFn(void* ptr, size_t size, size_t nmemb, void* stream)
{
	char* pszHeader = (char*)ptr;
	char* pszValue = NULL;
	RequestContext_t* pRC = (RequestContext_t*)stream;

	pszHeader[(size * nmemb - 1)] = NULL;
	pszValue = Q_strstr(pszHeader, ":");
	if (pszValue)
	{
		// null terminate the header name, and point pszValue at it's value
		*pszValue = NULL;
		pszValue++;
		Q_StrTrim(pszValue);
	}
	if (0 == Q_stricmp(pszHeader, "Content-Length"))
	{
		size_t len = atol(pszValue);
		if (pRC && len)
		{
			pRC->nBytesTotal = len;
			pRC->data = (byte*)malloc(len);
		}
	}

	return size * nmemb;
}


// we're going to abuse this by using it for proxy pac fetching
// the cacheData field will hold the URL of the PAC, and the data
// field the contents of the pac
RequestContext_t g_pacRequestCtx;


void SetProxiesForURL(CURL* hMasterCURL, const char* pszURL)
{
	uint32 uProxyPort = 0;
	char rgchProxyHost[1024];
	char* pszProxyExceptionList = NULL;
	rgchProxyHost[0] = '\0';

		if (rgchProxyHost[0] == '\0' || uProxyPort <= 0)
		{
			if (pszProxyExceptionList)
				free(pszProxyExceptionList);
			return;
		}

	curl_easy_setopt(hMasterCURL, CURLOPT_PROXY, rgchProxyHost);
	curl_easy_setopt(hMasterCURL, CURLOPT_PROXYPORT, uProxyPort);
	if (pszProxyExceptionList)
	{
		curl_easy_setopt(hMasterCURL, (CURLoption)(CURLOPTTYPE_OBJECTPOINT + 177) /*CURLOPT_NOPROXY*/, pszProxyExceptionList);
		free(pszProxyExceptionList);
	}

}


void DownloadThread(void* voidPtr)
{
	//static bool bDoneInit = false;
	//if (!bDoneInit)
	//{
	//	bDoneInit = true;
	//	curl_global_init(CURL_GLOBAL_SSL);
	//}

	RequestContext_t& rc = *(RequestContext_t*)voidPtr;


	rc.status = HTTP_FETCH;

	CURL* hCURL = curl_easy_init();
	if (!hCURL)
	{
		rc.error = HTTP_ERROR_INVALID_URL;
		rc.status = HTTP_ERROR;
		rc.threadDone = true;
		return;
	}




	// turn off certificate verification similar to how we set INTERNET_FLAG_IGNORE_CERT_CN_INVALID and INTERNET_FLAG_IGNORE_CERT_DATE_INVALID on Windows



	// and now the callback fns
	curl_easy_setopt(hCURL, CURLOPT_HEADERFUNCTION, &curlHeaderFn);
	curl_easy_setopt(hCURL, CURLOPT_WRITEFUNCTION, &curlWriteFn);
	curl_easy_setopt(hCURL, CURLOPT_FOLLOWLOCATION, 1);





	// setup proxies
	//SetProxiesForURL(hCURL, pchURL);

	// set the url
	curl_easy_setopt(hCURL, CURLOPT_URL, rc.baseURL);

	// setup callback stream pointers
	curl_easy_setopt(hCURL, CURLOPT_WRITEHEADER, &rc);
	curl_easy_setopt(hCURL, CURLOPT_WRITEDATA, &rc);

	//curl_easy_setopt(hCURL, CURLOPT_FOLLOWLOCATION, 1);
	//curl_easy_setopt(hCURL, CURLOPT_MAXREDIRS, 2);
	//curl_easy_setopt(hCURL, CURLOPT_UNRESTRICTED_AUTH, 1);
	curl_easy_setopt(hCURL, CURLOPT_USERAGENT, "SourceBox_" DEST_OS);



	// g0g0g0
	CURLcode res = curl_easy_perform(hCURL);



	if (res == CURLE_OK)
	{
		curl_easy_getinfo(hCURL, CURLINFO_RESPONSE_CODE, &rc.status);
		Msg("%i\n", rc.status);
		if (rc.status == HTTPStatus_t(200) || rc.status == HTTPStatus_t(206))
		{
			rc.status = HTTP_DONE;
			rc.error = HTTP_ERROR_NONE;
		}
		else
		{
			rc.status = HTTP_ERROR;
			rc.error = HTTP_ERROR_FILE_NONEXISTENT;
		}
	}
	else
	{
		rc.status = HTTP_ERROR;
	}

	// wait until the main thread says we can go away (so it can look at rc.data).
	while (!rc.shouldStop)
	{
		ThreadSleep(100);
	}

	// Delete rc.data, which was allocated in this thread
	if (rc.data != NULL)
	{
		delete[] rc.data;
		rc.data = NULL;
	}


	curl_easy_cleanup(hCURL);

	rc.threadDone = true;
}


CHttpDownloader::CHttpDownloader(IDownloadHandler* pHandler)
	: m_pHandler(pHandler),
	m_flNextThinkTime(0.0f),
	m_uBytesDownloaded(0),
	m_uSize(0),
	m_pThreadState(NULL),
	m_bDone(false),
	m_nHttpError(HTTP_ERROR_NONE),
	m_nHttpStatus(HTTP_INVALID),
	m_pBytesDownloaded(NULL),
	m_pUserData(NULL)
{
}

CHttpDownloader::~CHttpDownloader()
{
	CleanupThreadIfDone();
}

bool CHttpDownloader::CleanupThreadIfDone()
{
	if (!m_pThreadState || !m_pThreadState->threadDone)
		return false;

	// NOTE: The context's "data" member will have already been cleaned up by the
	// download thread at this point.
	delete m_pThreadState;
	m_pThreadState = NULL;

	return true;
}

bool CHttpDownloader::BeginDownload(const char* pURL, const char* pGamePath, void* pUserData, uint32* pBytesDownloaded)
{
	Msg("%s\n", pURL);
	if (!pURL || !pURL[0])
		return false;

	m_pThreadState = new RequestContext_t();
	if (!m_pThreadState)
		return false;

	// Cache any user data
	m_pUserData = pUserData;

	// Cache bytes downloaded
	m_pBytesDownloaded = pBytesDownloaded;

	// Setup request context
	m_pThreadState->bAsHTTP = false;


	// Cache URL - for debugging
	V_strcpy(m_szURL, pURL);
	V_strcpy(m_pThreadState->baseURL, pURL);

	// Spawn the download thread
	uintp nThreadID;
	VCRHook_CreateThread(NULL, 0,
#ifdef POSIX
	(void*)
#endif
		DownloadThread, m_pThreadState, 0, (uintp*)&nThreadID);

	//ReleaseThreadHandle((ThreadHandle_t)nThreadID);
	return nThreadID != 0;
}

void CHttpDownloader::AbortDownloadAndCleanup()
{
	// Make sure that this function isn't executed simultaneously by
	// multiple threads in order to avoid use-after-free crashes during
	// shutdown.
	AUTO_LOCK(m_lock);

	if (!m_pThreadState)
		return;

	// Thread already completed?
	if (m_pThreadState->threadDone)
	{
		CleanupThreadIfDone();
		return;
	}

	// Loop until the thread cleans up
	m_pThreadState->shouldStop = true;
	while (!m_pThreadState->threadDone)
		;

	// Cache state for handler
	m_nHttpError = m_pThreadState->error;
	m_nHttpStatus = HTTP_ABORTED;	// Force this to be safe
	m_uBytesDownloaded = 0;
	m_uSize = m_pThreadState->nBytesTotal;
	m_bDone = true;

	InvokeHandler();
	CleanupThreadIfDone();
}

void CHttpDownloader::Think()
{
	const float flHostTime = ((float)(g_pVGuiSystem->GetTimeMillis()))/1000.0f;
	if (m_flNextThinkTime > flHostTime)
		return;

	if (!m_pThreadState)
		return;

	// If thread is done, cleanup now
	if (CleanupThreadIfDone())
		return;

	// If we haven't already set shouldStop, check the download status
	if (!m_pThreadState->shouldStop)
	{
		// Security measure: make sure the file size isn't outrageous
		const bool bEvilFileSize = m_pThreadState->nBytesTotal &&
			m_pThreadState->nBytesTotal >= DOWNLOAD_MAX_SIZE;
#if _DEBUG
		extern ConVar replay_simulate_evil_download_size;
		if (replay_simulate_evil_download_size.GetBool() || bEvilFileSize)
#else
		if (bEvilFileSize)
#endif
		{
			AbortDownloadAndCleanup();
			return;
		}

		bool bConnecting = false;	// For fall-through in HTTP_CONNECTING case.

#if _DEBUG
		extern ConVar replay_simulatedownloadfailure;
		if (replay_simulatedownloadfailure.GetInt() == 1)
		{
			m_pThreadState->status = HTTP_ERROR;
		}
#endif

		switch (m_pThreadState->status)
		{
		case HTTP_CONNECTING:

			// Call connecting handler
			if (m_pHandler)
			{
				m_pHandler->OnConnecting(this);
			}

			bConnecting = true;

			// Fall-through

		case HTTP_FETCH:

			m_uBytesDownloaded = (uint32)m_pThreadState->nBytesCurrent;
			m_uSize = m_pThreadState->nBytesTotal;

			Assert(m_uBytesDownloaded <= m_uSize);

			// Call fetch handle
			if (!bConnecting && m_pHandler)
			{
				m_pHandler->OnFetch(this);
			}

			break;

		case HTTP_ABORTED:
		case HTTP_DONE:
		case HTTP_ERROR:

			// Cache state
			m_nHttpError = m_pThreadState->error;
			m_nHttpStatus = m_pThreadState->status;
			m_uBytesDownloaded = (uint32)m_pThreadState->nBytesCurrent;
			m_uSize = m_pThreadState->nBytesTotal;	// NOTE: Need to do this here in the case that a file is small enough that we never hit HTTP_FETCH
			m_bDone = true;

			// Call handler
			InvokeHandler();

			// Tell the thread to cleanup so we can free it
			m_pThreadState->shouldStop = true;

			break;
		}
	}

	// Write bytes for user if changed
	if (m_pBytesDownloaded && *m_pBytesDownloaded != m_uBytesDownloaded)
	{
		*m_pBytesDownloaded = m_uBytesDownloaded;
	}

	// Set next think time
	m_flNextThinkTime = flHostTime + 0.1f;
}

void CHttpDownloader::InvokeHandler()
{
	if (!m_pHandler)
		return;

	// NOTE: Don't delete the downloader in OnDownloadComplete()!
	m_pHandler->OnDownloadComplete(this, m_pThreadState->data);
}

void UpdaterDownloadHandler::OnConnecting(CHttpDownloader* pDownloader)
{
	return;
}

void UpdaterDownloadHandler::OnFetch(CHttpDownloader* pDownloader)
{
	g_pRootPanel->SetProgress((float)(pDownloader->GetBytesDownloaded()) / (float)(pDownloader->GetSize())*0.1);
}

void UpdaterDownloadHandler::OnDownloadComplete(CHttpDownloader* pDownloader, const unsigned char* pData)
{
	g_pRootPanel->SetProgress(0.1);
	KeyValuesJSONParser* parser = new KeyValuesJSONParser((const char*)pData,pDownloader->GetBytesDownloaded());
	KeyValues* keyvalues = parser->ParseFile();
	if (!keyvalues)
	{
		::MessageBoxA(0, parser->m_szErrMsg, "Updater error", MB_OK);
		exit(1);
		return;
	}
	KeyValues* assets = keyvalues->FindKey("assets");
	if (!assets)
		return;
	for (KeyValues* asset = assets->GetFirstTrueSubKey(); asset;asset = asset->GetNextTrueSubKey())
	{
		char ending[sizeof(DEST_OS ".7z")];
		V_StrRight(asset->GetString("name"), sizeof(DEST_OS ".7z")-1, ending, sizeof(DEST_OS ".7z"));
		if (strcmp(ending, DEST_OS ".7z") == 0)
		{
			V_strncpy(FileUrl,asset->GetString("browser_download_url"),sizeof(FileUrl));
			g_pFileDownloader->BeginDownload(FileUrl);
			break;
		}
	}
}


void GameDownloadHandler::OnConnecting(CHttpDownloader* pDownloader)
{
	return;
}

void GameDownloadHandler::OnFetch(CHttpDownloader* pDownloader)
{
	g_pRootPanel->SetProgress((float)(pDownloader->GetBytesDownloaded()) / (float)(pDownloader->GetSize()) * 0.9 + 0.1);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const char
//-----------------------------------------------------------------------------
const wchar* GetBaseDirectory(void)
{
	static char path[MAX_PATH] = { 0 };
	static wchar wpath[MAX_PATH] = { 0 };
	if (path[0] == 0)
	{
		GetModuleFileName((HMODULE)GetAppInstance(), path, sizeof(path));
		Q_StripLastDir(path, sizeof(path));	// Get rid of the filename.
		V_strtowcs(path, MAX_PATH, wpath, MAX_PATH<<1);
	}
	return wpath;
}

void GameDownloadHandler::OnDownloadComplete(CHttpDownloader* pDownloader, const unsigned char* pData)
{
	g_pRootPanel->SetProgress(1.0);
	try {
		bit7z::Bit7zLibrary shit(L"7zxa.dll");
		bit7z::BitMemExtractor* extracter = new bit7z::BitMemExtractor(shit, bit7z::BitFormat::SevenZip);
		std::vector<bit7z::byte_t> fuck(pData, pData + pDownloader->GetBytesDownloaded());
		std::wstring bitch(GetBaseDirectory());
		extracter->extract(fuck, bitch);
		
	}
	catch (const bit7z::BitException& ex)
	{
		::MessageBoxA(0, ex.what(),"Updater error", MB_OK);
		exit(0);
		return;
	}
	
}