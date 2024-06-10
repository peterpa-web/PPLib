#pragma once
#include <PortableDeviceApi.h>  // Include this header for Windows Portable Device API interfaces
#include <PortableDevice.h>     // Include this header for Windows Portable Device definitions

#ifndef IID_PPV_ARGS
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), (static_cast<IUnknown *>(*(ppType)),reinterpret_cast<void**>(ppType))
#endif

class CPortableDevice
{
protected:
	CComPtr<IPortableDevice> m_pDevice = nullptr;
	CComPtr<IPortableDeviceManager> m_pPortableDeviceManager = nullptr;
	CString m_strLastObjId;
	CString m_strLastDirObjId;

	void GetClientInformation(
		IPortableDeviceValues** ppClientInformation);
	CStringW GetDeviceDescr(PCWSTR pPnPDeviceID);
	CStringW GetObjectName(IPortableDeviceContent* pContent, PCWSTR pszObjectID, DATE* pDate);
	HRESULT StreamCopy(
		IStream*	pDestStream,
		IStream*	pSourceStream,
		DWORD       cbTransferSize,
		DWORD*		pcbWritten);
	HRESULT GetRequiredPropertiesForAllContentTypes(
		IPortableDeviceValues* pObjectProperties,
		PCWSTR                  pszParentObjectID,
		PCWSTR                  pszFilePath,
		IStream* pFileStream);
	HRESULT GetRequiredPropertiesForContentType(
		REFGUID                 ContentType,
		PCWSTR                  pszParentObjectID,
		PCWSTR                  pszFilePath,
		IStream*				pFileStream,
		IPortableDeviceValues** ppObjectProperties);
	bool SupportsCommand(PROPERTYKEY theKey);

public:
	bool Open(const CStringW& strDevName);
	bool IsOpen() { return m_pDevice != nullptr; }
	void Close();
	CStringW GetChildObjIdByName(PCWSTR pszObjectID, const CStringW& strName, DATE* pDate = nullptr);
	CStringW GetObjIdFromPath(const CStringW& strDevPath, DATE* pDate = nullptr);
	CString GetLastObjId() { return m_strLastObjId; }
	CString GetLastDirObjId() { return m_strLastDirObjId; }
	void TransferContentFromDevice(PCWSTR pszObjectID, const CString& strLocalPathName);
	void TransferContentToDevice(PCWSTR pszDirObjectID, const CString& strLocalPathName);
	void RenameObject(PCWSTR objectId, PCWSTR newName);
	void DeleteContentFromDevice(PCWSTR pszObjectID);
};

class CPortableDeviceException : public CException
{
public:
	DECLARE_DYNAMIC(CPortableDeviceException)
	CPortableDeviceException(LPCTSTR lpszErrorText, BOOL bAutoDelete = TRUE);
	CPortableDeviceException(LPCTSTR lpszErrorText, HRESULT hr, BOOL bAutoDelete = TRUE);
	virtual BOOL GetErrorMessage(
		LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext) const;

protected:
	CString m_strErrorText;
};
