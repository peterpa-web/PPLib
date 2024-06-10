#include "stdafx.h"
#include "PortableDevicePP.h"

// see https://learn.microsoft.com/en-us/windows/win32/wpd_sdk/wpdapisample-sample-application

#pragma comment( lib,"PortableDeviceGuids" )

#define CLIENT_NAME         L"WPD MFC Util"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     1
// This number controls how many object identifiers are requested during each call
// to IEnumPortableDeviceObjectIDs::Next()
#define NUM_OBJECTS_TO_REQUEST  10


// Creates and populates an IPortableDeviceValues with information about
// this application.  The IPortableDeviceValues is used as a parameter
// when calling the IPortableDevice::Open() method.
void CPortableDevice::GetClientInformation(
    IPortableDeviceValues** ppClientInformation)
{
    // Client information is optional.  The client can choose to identify itself, or
    // to remain unknown to the driver.  It is beneficial to identify yourself because
    // drivers may be able to optimize their behavior for known clients. (e.g. An
    // IHV may want their bundled driver to perform differently when connected to their
    // bundled software.)

    // CoCreate an IPortableDeviceValues interface to hold the client information.
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(ppClientInformation));
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to CoCreateInstance CLSID_PortableDeviceValues", hr);

    // Attempt to set all bits of client information
    hr = (*ppClientInformation)->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to set WPD_CLIENT_NAME", hr);

    hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to set WPD_CLIENT_MAJOR_VERSION", hr);

    hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to set WPD_CLIENT_MINOR_VERSION", hr);

    hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to set WPD_CLIENT_REVISION", hr);

    //  Some device drivers need to impersonate the caller in order to function correctly.  Since our application does not
    //  need to restrict its identity, specify SECURITY_IMPERSONATION so that we work with all devices.
    hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to set WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE", hr);
}

CStringW CPortableDevice::GetDeviceDescr(PCWSTR pPnPDeviceID)
{
    DWORD    cchDescription = 0;
    CStringW strDescription;
    
    // First, pass NULL as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    HRESULT hr = m_pPortableDeviceManager->GetDeviceDescription(pPnPDeviceID, NULL, &cchDescription);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to get number of characters for device description", hr);

    // Second allocate the number of characters needed and retrieve the string value.
    if ((hr == S_OK) && (cchDescription > 0))
    {
        LPWSTR pszDescription = strDescription.GetBufferSetLength(cchDescription);
        hr = m_pPortableDeviceManager->GetDeviceDescription(pPnPDeviceID, pszDescription, &cchDescription);
        strDescription.ReleaseBufferSetLength(cchDescription);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"Failed to get device description", hr);
    }

    if (SUCCEEDED(hr) && (cchDescription == 0))
    {
        TRACE0("The device did not provide a description.\n");
    }
    return strDescription;
}

CStringW CPortableDevice::GetObjectName(IPortableDeviceContent* pContent, PCWSTR pszObjectID, DATE* pDate)
{
    CString strResult;
    HRESULT                               hr = S_OK;
    CComPtr<IPortableDeviceProperties>    pProperties;
    CComPtr<IPortableDeviceValues>        pObjectProperties;
    CComPtr<IPortableDeviceKeyCollection> pPropertiesToRead;

    // 2) Get an IPortableDeviceProperties interface from the IPortableDeviceContent interface
    // to access the property-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"Failed to get IPortableDeviceProperties from IPortableDevice", hr);
    }

    // 3) CoCreate an IPortableDeviceKeyCollection interface to hold the the property keys
    // we wish to read.
    //<SnippetContentProp1>
    hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pPropertiesToRead));
    if (SUCCEEDED(hr))
    {
        // 4) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
        // NOTE: We are not handling any special error cases here so we can proceed with
        // adding as many of the target properties as we can.
        if (pPropertiesToRead != nullptr)
        {
            HRESULT hrTemp = S_OK;
            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_NAME);
            if (FAILED(hrTemp))
                throw new CPortableDeviceException(L"Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection", hr);

            hrTemp = pPropertiesToRead->Add(WPD_OBJECT_DATE_MODIFIED);
            if (FAILED(hrTemp))
                throw new CPortableDeviceException(L"Failed to add WPD_OBJECT_DATE_MODIFIED to IPortableDeviceKeyCollection", hr);
        }
    }
    // 5) Call GetValues() passing the collection of specified PROPERTYKEYs.
    if (SUCCEEDED(hr))
    {
        hr = pProperties->GetValues(pszObjectID,         // The object whose properties we are reading
            pPropertiesToRead,   // The properties we want to read
            &pObjectProperties); // Driver supplied property values for the specified object
        if (FAILED(hr))
            throw new CPortableDeviceException(L"Failed to get all object properties", hr);
    }
    // 6) Fetch the returned property values
    if (SUCCEEDED(hr))
    {
        // Fetch WPD_OBJECT_NAME
        PWSTR   pszValue = nullptr;
        HRESULT hr = pObjectProperties->GetStringValue(WPD_OBJECT_NAME, &pszValue);
        if (SUCCEEDED(hr))
        {
            strResult = pszValue;
            if (pDate != nullptr)
            {
                *pDate = (DATE)0;
                PROPVARIANT   propValue = { 0 };
                HRESULT hr = pObjectProperties->GetValue(WPD_OBJECT_DATE_MODIFIED, &propValue);
                if (SUCCEEDED(hr))
                {
                    if (propValue.vt != VT_DATE)
                    {
                        TRACE0("No VT_DATE\n");
                    }
                    else
                        *pDate = propValue.date;
                }
            }
        }
        else
        {
            //printf("%ws: <Not Found>\n", pszKey);
            strResult = L"???";
        }

        // Free the allocated string returned from the
        // GetStringValue method
        CoTaskMemFree(pszValue);
        pszValue = nullptr;
    }
    return strResult;
}

// Function which enumerates using the specified object id as the parent.
// Returns the object id for the matching name and the modif date if a valid pointer is given
CStringW CPortableDevice::GetChildObjIdByName(
    PCWSTR                  pszObjectID,
    const CStringW&         strName,
    DATE*                   pDate)
{
    CComPtr<IPortableDeviceContent> pContent = nullptr;
    CStringW strResult;

    // Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    HRESULT hr = m_pDevice->Content(&pContent);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to get IPortableDeviceContent from IPortableDevice", hr);

    CComPtr<IEnumPortableDeviceObjectIDs> pEnumObjectIDs;

    // Print the object identifier being used as the parent during enumeration.
    // printf("Parent: %ws\n", pszObjectID);

    // Get an IEnumPortableDeviceObjectIDs interface by calling EnumObjects with the
    // specified parent object identifier.
    hr = pContent->EnumObjects(0,               // Flags are unused
        pszObjectID,     // Starting from the passed in object
        NULL,            // Filter is unused
        &pEnumObjectIDs);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Failed to get IEnumPortableDeviceObjectIDs from IPortableDeviceContent", hr);

    // Loop calling Next() while S_OK is being returned.
    while (hr == S_OK && strResult.IsEmpty())
    {
        DWORD  cFetched = 0;
        PWSTR  szObjectIDArray[NUM_OBJECTS_TO_REQUEST] = { 0 };
        hr = pEnumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST,   // Number of objects to request on each NEXT call
            szObjectIDArray,          // Array of PWSTR array which will be populated on each NEXT call
            &cFetched);               // Number of objects written to the PWSTR array
        if (SUCCEEDED(hr))
        {
            // Traverse the results of the Next() operation and recursively enumerate
            // Remember to free all returned object identifiers using CoTaskMemFree()
            for (DWORD dwIndex = 0; dwIndex < cFetched; dwIndex++)
            {
                // printf("%ws\n", szObjectIDArray[dwIndex]);
                DATE dI;
                CStringW strNameI = GetObjectName(pContent, szObjectIDArray[dwIndex], &dI);
                int c = strNameI.Compare(strName);
                if (c == 0)
                {
                    strResult = szObjectIDArray[dwIndex];
                    if (pDate != nullptr)
                        *pDate = dI;
                }

                // Free allocated PWSTRs after the recursive enumeration call has completed.
                CoTaskMemFree(szObjectIDArray[dwIndex]);
                szObjectIDArray[dwIndex] = NULL;
            }
        }
    }
    return strResult;
}

CStringW CPortableDevice::GetObjIdFromPath(const CStringW& strDevPath, DATE* pDate)
{
    CStringW strResult = WPD_DEVICE_OBJECT_ID;
    int iStart = 0;
    CStringW str = strDevPath.Tokenize(L"/\\", iStart);
    while (str.GetLength() != 0)
    {
        m_strLastDirObjId = strResult;
        strResult = GetChildObjIdByName(strResult, str, pDate);
        if (strResult.IsEmpty())
        {
            TRACE1("GetObjIdFromPath failed with %s\n", str);
            break;
        }
        str = strDevPath.Tokenize(L"/\\", iStart);
    }
    m_strLastObjId = strResult;
    return strResult;
}

bool CPortableDevice::Open(const CStringW& strDevName)
{
    DWORD cPnPDeviceIDs = 0;
    PWSTR* pPnpDeviceIDs = NULL;
    CComPtr<IPortableDeviceValues>  pClientInformation = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pPortableDeviceManager));
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Open: Failed to CoCreateInstance CLSID_PortableDeviceManager", hr);

    if (IsOpen())
    {
        throw new CPortableDeviceException(L"Open: Is already opened.");
        return false;
    }

    // Fill out information about your application, so the device knows
    // who they are speaking to.

    GetClientInformation(&pClientInformation);

    // First, pass NULL as the PWSTR array pointer to get the total number
    // of devices found on the system.
    if (SUCCEEDED(hr))
    {
        hr = m_pPortableDeviceManager->GetDevices(NULL, &cPnPDeviceIDs);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"Open: Failed to get number of devices on the system", hr);
    }
    // Second, allocate an array to hold the PnPDeviceID strings returned from
    // the IPortableDeviceManager::GetDevices method
    if (SUCCEEDED(hr) && (cPnPDeviceIDs > 0))
    {
        pPnpDeviceIDs = new (std::nothrow) PWSTR[cPnPDeviceIDs];
        if (pPnpDeviceIDs == nullptr)
        {
            throw new CPortableDeviceException(L"Open: Failed to allocate memory for PWSTR array.");
            return false;
        }

        DWORD dwIndex = 0;
        CPortableDeviceException* ppde = nullptr;
        try {
            hr = m_pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &cPnPDeviceIDs);
            if (SUCCEEDED(hr))
            {
                // For each device found, check the devices description
                for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
                {
                    CString strDescr = GetDeviceDescr(pPnpDeviceIDs[dwIndex]);
                    int c = strDescr.Compare(strDevName);
                    if (c == 0)
                    {
                        // CoCreate the IPortableDevice interface and call Open() with
                        // the chosen PnPDeviceID string.
                        hr = CoCreateInstance(CLSID_PortableDeviceFTM,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_PPV_ARGS(&m_pDevice));
                        if (SUCCEEDED(hr))
                        {
                            hr = (m_pDevice)->Open(pPnpDeviceIDs[dwIndex], pClientInformation);
                            if (FAILED(hr))
                            {
                                if (hr == E_ACCESSDENIED)
                                {
                                    TRACE0("Open: Failed to Open the device for Read Write access, will open it for Read-only access instead\n");
                                    pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
                                    hr = m_pDevice->Open(pPnpDeviceIDs[dwIndex], pClientInformation);
                                    if (FAILED(hr))
                                    {
                                        // Release the IPortableDevice interface, because we cannot proceed
                                        // with an unopen device.
                                        Close();
                                        throw new CPortableDeviceException(L"Failed to Open the device", hr);
                                    }
                                }
                                else
                                {
                                    // Release the IPortableDevice interface, because we cannot proceed
                                    // with an unopen device.
                                    Close();
                                    throw new CPortableDeviceException(L"Failed to Open the device", hr);
                                }
                            }
                        }
                        else
                        {
                            throw new CPortableDeviceException(L"Open: Failed to CoCreateInstance CLSID_PortableDeviceFTM", hr);
                        }
                        break;
                    }
                }
            }
            else
            {
                throw new CPortableDeviceException(L"Open: Failed to get the device list from the system", hr);
            }
        }
        catch (CPortableDeviceException* pe)
        {
            ppde = pe;
        }

        // Free all returned PnPDeviceID strings by using CoTaskMemFree.
        // NOTE: CoTaskMemFree can handle NULL pointers, so no NULL
        //       check is needed.
        for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
        {
            CoTaskMemFree(pPnpDeviceIDs[dwIndex]);
            pPnpDeviceIDs[dwIndex] = nullptr;
        }

        // Delete the array of PWSTR pointers
        delete[] pPnpDeviceIDs;
        pPnpDeviceIDs = nullptr;

        if (ppde != nullptr)
            throw ppde;
    }
    return m_pDevice != nullptr;
}

void CPortableDevice::Close()
{
    m_pPortableDeviceManager = nullptr;
    m_pDevice = NULL;
}

// Copies data from a source stream to a destination stream using the
// specified cbTransferSize as the temporary buffer size.
HRESULT CPortableDevice::StreamCopy(
    IStream* pDestStream,
    IStream* pSourceStream,
    DWORD    cbTransferSize,
    DWORD* pcbWritten)
{

    HRESULT hr = S_OK;

    // Allocate a temporary buffer (of Optimal transfer size) for the read results to
    // be written to.
    BYTE* pObjectData = new (std::nothrow) BYTE[cbTransferSize];
    if (pObjectData != NULL)
    {
        DWORD cbTotalBytesRead = 0;
        DWORD cbTotalBytesWritten = 0;

        DWORD cbBytesRead = 0;
        DWORD cbBytesWritten = 0;

        // Read until the number of bytes returned from the source stream is 0, or
        // an error occured during transfer.
        do
        {
            // Read object data from the source stream
            hr = pSourceStream->Read(pObjectData, cbTransferSize, &cbBytesRead);
            if (FAILED(hr))
            {
                TRACE2("! StreamCopy: Failed to read %d bytes from the source stream, hr = 0x%lx\n", cbTransferSize, hr);
            }

            // Write object data to the destination stream
            if (SUCCEEDED(hr))
            {
                cbTotalBytesRead += cbBytesRead; // Calculating total bytes read from device for debugging purposes only

                hr = pDestStream->Write(pObjectData, cbBytesRead, &cbBytesWritten);
                if (FAILED(hr))
                {
                    TRACE2("! StreamCopy: Failed to write %d bytes of object data to the destination stream, hr = 0x%lx\n", cbBytesRead, hr);
                }

                if (SUCCEEDED(hr))
                {
                    cbTotalBytesWritten += cbBytesWritten; // Calculating total bytes written to the file for debugging purposes only
                }
            }

            // Output Read/Write operation information only if we have received data and if no error has occured so far.
            if (SUCCEEDED(hr) && (cbBytesRead > 0))
            {
                TRACE2("StreamCopy: Read %d bytes from the source stream...Wrote %d bytes to the destination stream...\n", cbBytesRead, cbBytesWritten);
            }

        } while (SUCCEEDED(hr) && (cbBytesRead > 0));

        // If the caller supplied a pcbWritten parameter and we
        // and we are successful, set it to cbTotalBytesWritten
        // before exiting.
        if ((SUCCEEDED(hr)) && (pcbWritten != NULL))
        {
            *pcbWritten = cbTotalBytesWritten;
        }

        // Remember to delete the temporary transfer buffer
        delete[] pObjectData;
        pObjectData = NULL;
    }
    else
    {
        TRACE1("! StreamCopy: Failed to allocate %d bytes for the temporary transfer buffer.\n", cbTransferSize);
        throw new CPortableDeviceException(L"StreamCopy: Failed to allocate the transfer buffer.");
    }

    return hr;
}

// Transfers the object's data (WPD_RESOURCE_DEFAULT) to he given file.
void CPortableDevice::TransferContentFromDevice(
    PCWSTR pszObjectID, const CString& strLocalPathName)
{
    //<SnippetTransferFrom1>
    HRESULT                            hr = S_OK;
    CComPtr<IPortableDeviceContent>    pContent;
    CComPtr<IPortableDeviceResources>  pResources;
//    CComPtr<IPortableDeviceProperties> pProperties;
    CComPtr<IPortableDeviceKeyCollection> pKeys;
    CComPtr<IStream>                   pObjectDataStream;
    CComPtr<IStream>                   pFinalFileStream;
    DWORD                              cbOptimalTransferSize = 0;

    if (!IsOpen())
        throw new CPortableDeviceException(L"TransferContentFromDevice: Device was not opened.");

    //</SnippetTransferFrom1>
    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    //<SnippetTransferFrom2>
    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            throw new CPortableDeviceException(L"TransferContentFromDevice: Failed to get IPortableDeviceContent from IPortableDevice.", hr);
        }
    }
    //</SnippetTransferFrom2>
    // 2) Get an IPortableDeviceResources interface from the IPortableDeviceContent interface to
    // access the resource-specific methods.
    //<SnippetTransferFrom3>
    if (SUCCEEDED(hr))
    {
        hr = pContent->Transfer(&pResources);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"TransferContentFromDevice: Failed to get IPortableDeviceResources from IPortableDeviceContent.", hr);
    }
    //</SnippetTransferFrom3>
    // 3) Get the IStream (with READ access) and the optimal transfer buffer size
    // to begin the transfer.
    //<SnippetTransferFrom4>
    if (SUCCEEDED(hr))
    {
        hr = pResources->GetStream(pszObjectID,             // Identifier of the object we want to transfer
            WPD_RESOURCE_DEFAULT,    // We are transferring the default resource (which is the entire object's data)
            STGM_READ,               // Opening a stream in READ mode, because we are reading data from the device.
            &cbOptimalTransferSize,  // Driver supplied optimal transfer size
            &pObjectDataStream);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"TransferContentFromDevice: Failed to get IStream (representing object data on the device) from IPortableDeviceResources.", hr);
    }
    //</SnippetTransferFrom4>

    // 5) Create a destination for the data to be written to.  In this example we are
    // creating a temporary file which is named the same as the object identifier string.
    //<SnippetTransferFrom6>
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFile(strLocalPathName, STGM_CREATE | STGM_WRITE, &pFinalFileStream);
        if (FAILED(hr))
        {
            TRACE3("! TransferContentFromDevice: Failed to create the file named (%ws) to transfer object (%ws), hr = 0x%lx\n", (PCWSTR)strLocalPathName, pszObjectID, hr);
            throw new CPortableDeviceException(L"TransferContentFromDevice: Failed to create the target file.", hr);
        }
    }
    //</SnippetTransferFrom6>
    // 6) Read on the object's data stream and write to the final file's data stream using the
    // driver supplied optimal transfer buffer size.
    //<SnippetTransferFrom7>
    if (SUCCEEDED(hr))
    {
        DWORD cbTotalBytesWritten = 0;

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        hr = StreamCopy(pFinalFileStream,       // Destination (The Final File to transfer to)
            pObjectDataStream,      // Source (The Object's data to transfer from)
            cbOptimalTransferSize,  // The driver specified optimal transfer buffer size
            &cbTotalBytesWritten);  // The total number of bytes transferred from device to the finished file
        if (FAILED(hr))
            throw new CPortableDeviceException(L"TransferContentFromDevice: Failed to transfer object from device.", hr);
        else
        {
            TRACE2("* Transferred object '%ws' to '%ws'.\n", pszObjectID, (PCWSTR)strLocalPathName);
        }
    }
    //</SnippetTransferFrom7>
}

// Fills out the required properties for ALL content types...
HRESULT CPortableDevice::GetRequiredPropertiesForAllContentTypes(
    IPortableDeviceValues* pObjectProperties,
    PCWSTR                  pszParentObjectID,
    PCWSTR                  pszFilePath,
    IStream*                pFileStream)
{
    // Set the WPD_OBJECT_PARENT_ID
    HRESULT hr = pObjectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, pszParentObjectID);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"GetRequiredPropertiesForAllContentTypes: Failed to set WPD_OBJECT_PARENT_ID.", hr);

    // Set the WPD_OBJECT_SIZE by requesting the total size of the
    // data stream.
    if (SUCCEEDED(hr))
    {
        STATSTG statstg = { 0 };
        hr = pFileStream->Stat(&statstg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
            if (FAILED(hr))
                throw new CPortableDeviceException(L"GetRequiredPropertiesForAllContentTypes: Failed to set WPD_OBJECT_SIZE.", hr);
        }
        else
        {
            throw new CPortableDeviceException(L"GetRequiredPropertiesForAllContentTypes: Failed to get file's total size.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_ORIGINAL_FILE_NAME by splitting the file path
        // into a separate filename.
        WCHAR szFileName[MAX_PATH] = { 0 };
        WCHAR szFileExt[MAX_PATH] = { 0 };
        if (_wsplitpath_s(pszFilePath, NULL, 0, NULL, 0,
            szFileName, ARRAYSIZE(szFileName),
            szFileExt, ARRAYSIZE(szFileExt)))
        {
            throw new CPortableDeviceException(L"GetRequiredPropertiesForAllContentTypes: Failed to split the file path.");
        }

        CStringW strOriginalFileName;
        if (SUCCEEDED(hr))
        {
            strOriginalFileName.Format(L"%ws%ws", szFileName, szFileExt);
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, strOriginalFileName);
            if (FAILED(hr))
                throw new CPortableDeviceException(L"GetRequiredPropertiesForAllContentTypes: Failed to set WPD_OBJECT_ORIGINAL_FILE_NAME.", hr);
        }

        // Set the WPD_OBJECT_NAME.  We are using the  file name without its file extension in this
        // example for the object's name.  The object name could be a more friendly name like
        // "This Cool Song" or "That Cool Picture".
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, strOriginalFileName);    // szFileName
            if (FAILED(hr))
                throw new CPortableDeviceException(L"GetRequiredPropertiesForAllContentTypes: Failed to set WPD_OBJECT_NAME.", hr);
        }
    }
    return hr;
}

// Fills out the required properties for specific WPD content types.
HRESULT CPortableDevice::GetRequiredPropertiesForContentType(
    REFGUID                 ContentType,
    PCWSTR                  pszParentObjectID,
    PCWSTR                  pszFilePath,
    IStream* pFileStream,
    IPortableDeviceValues** ppObjectProperties)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pObjectProperties));
    if (SUCCEEDED(hr))
    {
        if (pObjectProperties == NULL)
            throw new CPortableDeviceException(L"GetRequiredPropertiesForContentType: Failed to create property information.");
        else
        {
            // Fill out required properties for ALL content types
            hr = GetRequiredPropertiesForAllContentTypes(pObjectProperties,
                pszParentObjectID,
                pszFilePath,
                pFileStream);
            if (FAILED(hr))
                throw new CPortableDeviceException(L"GetRequiredPropertiesForContentType: Failed to get required properties common to ALL content types.", hr);
            if (SUCCEEDED(hr))
            {
                // Fill out required properties for specific content types.
                // NOTE: If the content type is unknown to this function then
                // only the required properties will be written.  This is enough
                // for transferring most generic content types.
                //

        //        if (IsEqualGUID(ContentType, WPD_CONTENT_TYPE_IMAGE))
        //        {
        //            hr = GetRequiredPropertiesForImageContentTypes(pObjectProperties);
        //        }
        //        else if (IsEqualGUID(ContentType, WPD_CONTENT_TYPE_AUDIO))
        //        {
        //            hr = GetRequiredPropertiesForMusicContentTypes(pObjectProperties);
        //        }
        //        else if (IsEqualGUID(ContentType, WPD_CONTENT_TYPE_CONTACT))
        //        {
        //            hr = GetRequiredPropertiesForContactContentTypes(pObjectProperties);
        //        }
            }

            // If everything was successful above, QI for the IPortableDeviceValues to return
            // to the caller.  A temporary CComPtr IPortableDeviceValues was used for easy cleanup
            // in case of a failure.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
                if (FAILED(hr))
                    throw new CPortableDeviceException(L"GetRequiredPropertiesForContentType: Failed to QueryInterface for IPortableDeviceValues.", hr);
            }
        }
    }

    return hr;
}

void CPortableDevice::TransferContentToDevice(PCWSTR pszDirObjectID, const CString& strLocalPathName)
{
    if (!IsOpen())
        throw new CPortableDeviceException(L"TransferContentToDevice: Device was not opened.");

    HRESULT                             hr = S_OK;
    DWORD                               cbOptimalTransferSize = 0;
    CComPtr<IStream>                    pFileStream;
    CComPtr<IPortableDeviceDataStream>  pFinalObjectDataStream;
    CComPtr<IPortableDeviceValues>      pFinalObjectProperties;
    CComPtr<IPortableDeviceContent>     pContent;
    CComPtr<IStream>                    pTempStream;  // Temporary IStream which we use to QI for IPortableDeviceDataStream

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    //<SnippetContentTransfer2>
    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->Content(&pContent);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"TransferContentToDevice: Failed to get IPortableDeviceContent from IPortableDevice.", hr);
    }
    // 3) Open the source file and add required properties about the file being transferred
    //<SnippetContentTransfer4>

    if (SUCCEEDED(hr))
    {
        // Open the selected file as an IStream.  This will simplify reading the
        // data and writing to the device.
        hr = SHCreateStreamOnFile(strLocalPathName, STGM_READ, &pFileStream);
        if (SUCCEEDED(hr))
        {
            // Get the required properties needed to properly describe the data being
            // transferred to the device.
            hr = GetRequiredPropertiesForContentType(WPD_CONTENT_TYPE_UNSPECIFIED,           // Content type of the data
                pszDirObjectID,              // Parent to transfer the data under
                strLocalPathName,               // Full file path to the data file
                pFileStream,               // Open IStream that contains the data
                &pFinalObjectProperties);  // Returned properties describing the data
            if (FAILED(hr))
                throw new CPortableDeviceException(L"TransferContentToDevice: Failed to get required properties needed to transfer a file to the device.", hr);
        }

        if (FAILED(hr))
        {
            TRACE2("! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n", strLocalPathName, hr);
            throw new CPortableDeviceException(L"TransferContentToDevice: Failed to open source file.", hr);
        }
    }
    //</SnippetContentTransfer4>
    //<SnippetContentTransfer5>
    // 4) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = pContent->CreateObjectWithPropertiesAndData(pFinalObjectProperties,    // Properties describing the object data
            &pTempStream,              // Returned object data stream (to transfer the data to)
            &cbOptimalTransferSize,    // Returned optimal buffer size to use during transfer
            NULL);

        // Once we have a the IStream returned from CreateObjectWithPropertiesAndData,
        // QI for IPortableDeviceDataStream so we can use the additional methods
        // to get more information about the object (i.e. The newly created object
        // identifier on the device)
        if (SUCCEEDED(hr))
        {
            hr = pTempStream->QueryInterface(IID_PPV_ARGS(&pFinalObjectDataStream));
            if (FAILED(hr))
                throw new CPortableDeviceException(L"TransferContentToDevice: Failed to QueryInterface for IPortableDeviceDataStream.", hr);
        }

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (FAILED(hr))
            throw new CPortableDeviceException(L"TransferContentToDevice: Failed to get IStream (representing destination object data on the device) from IPortableDeviceContent.", hr);
        if (SUCCEEDED(hr))
        {
            DWORD cbTotalBytesWritten = 0;

            hr = StreamCopy(pFinalObjectDataStream, // Destination (The Object to transfer to)
                pFileStream,            // Source (The File data to transfer from)
                cbOptimalTransferSize,  // The driver specified optimal transfer buffer size
                &cbTotalBytesWritten);  // The total number of bytes transferred from file to the device
            if (FAILED(hr))
                throw new CPortableDeviceException(L"TransferContentToDevice: Failed to transfer object to device.", hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = pFinalObjectDataStream->Commit(0);
            if (FAILED(hr))
                throw new CPortableDeviceException(L"TransferContentToDevice: Failed to commit object to device.", hr);
        }

        // Some clients may want to know the object identifier of the newly created
        // object.  This is done by calling GetObjectID() method on the
        // IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            PWSTR pszNewlyCreatedObject = NULL;
            hr = pFinalObjectDataStream->GetObjectID(&pszNewlyCreatedObject);
            if (SUCCEEDED(hr))
            {
                TRACE2("The file '%ws' was transferred to the device.\nThe newly created object's ID is '%ws'\n", strLocalPathName, pszNewlyCreatedObject);
            }

            // Free the object identifier string returned from the GetObjectID() method.
            CoTaskMemFree(pszNewlyCreatedObject);
            pszNewlyCreatedObject = NULL;

            if (FAILED(hr))
                throw new CPortableDeviceException(L"TransferContentToDevice: Failed to get the newly transferred object's identifier from the device.", hr);
        }
    }
}

bool CPortableDevice::SupportsCommand(PROPERTYKEY theKey)
{
    HRESULT hr = S_OK;
    CComPtr<IPortableDeviceCapabilities> capabilities;
    CComPtr<IPortableDeviceKeyCollection> commands;
    DWORD numCommands = 0;
    m_pDevice->Capabilities(&capabilities);
    hr = capabilities->GetSupportedCommands(&commands);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"SupportsCommand: GetSupportedCommands failed.", hr);
    hr = commands->GetCount(&numCommands);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"SupportsCommand: GetCount failed.", hr);
    PROPERTYKEY key = WPD_PROPERTY_NULL;
    for (DWORD index = 0; index < numCommands; index++) {
        hr = commands->GetAt(index, &key);
        if (FAILED(hr))
            throw new CPortableDeviceException(L"SupportsCommand: Get from commands failed.", hr);
        if (IsEqualPropertyKey(theKey, key))
            return true;
    }
    return false;
}

void CPortableDevice::RenameObject(PCWSTR objectId, PCWSTR newName)
{
    HRESULT hr = S_OK;
    if (SupportsCommand(WPD_COMMAND_OBJECT_PROPERTIES_SET) == false)
        throw new CPortableDeviceException(L"Rename: Not supported command.");
    CComPtr<IPortableDeviceValues> properties, values, results;
    hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (VOID**)&properties);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to create properties.", hr);
    hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (VOID**)&values);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to create values.", hr);
    // Mount the command.
    hr = properties->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_OBJECT_PROPERTIES_SET.fmtid);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to set fmtid.", hr);
    hr = properties->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_OBJECT_PROPERTIES_SET.pid);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to set pid.", hr);
    // Set the values
    hr = properties->SetStringValue(WPD_PROPERTY_OBJECT_PROPERTIES_OBJECT_ID, objectId);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to set objectId.", hr);
    hr = values->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, newName);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to set original file name.", hr);
    hr = properties->SetIPortableDeviceValuesValue(WPD_PROPERTY_OBJECT_PROPERTIES_PROPERTY_VALUES, values);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: Failed to set property values.", hr);
    hr = m_pDevice->SendCommand(0, properties, &results);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"Rename: SendCommand failed.", hr);
    // Show the results
//    DWORD count = 0;
//    results->GetCount(&count);
//    PROPERTYKEY key;
//    PROPVARIANT var;
//    for (DWORD i = 0; i < count; i++) {
//        results->GetAt(i, &key, &var);
        // ...show key and var...
//    }
}

// Deletes a selected object from the device.
void CPortableDevice::DeleteContentFromDevice(
    PCWSTR pszObjectID)
{
    HRESULT                                       hr = S_OK;
    CComPtr<IPortableDeviceContent>               pContent;
    CComPtr<IPortableDevicePropVariantCollection> pObjectsToDelete;
    CComPtr<IPortableDevicePropVariantCollection> pObjectsFailedToDelete;

    if (!IsOpen())
        throw new CPortableDeviceException(L"DeleteContentFromDevice: Device was not opened.");

    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    hr = m_pDevice->Content(&pContent);
    if (FAILED(hr))
        throw new CPortableDeviceException(L"DeleteContentFromDevice: Failed to get IPortableDeviceContent from IPortableDevice.", hr);

    // 2) CoCreate an IPortableDevicePropVariantCollection interface to hold the the object identifiers
    // to delete.
    //
    // NOTE: This is a collection interface so more than 1 object can be deleted at a time.
    //       This sample only deletes a single object.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pObjectsToDelete));
        if (FAILED(hr))
            throw new CPortableDeviceException(L"DeleteContentFromDevice: Failed to CoCreateInstance CLSID_PortableDevicePropVariantCollection.", hr);

        if (SUCCEEDED(hr))
        {
            if (pObjectsToDelete == NULL)
                throw new CPortableDeviceException(L"DeleteContentFromDevice: Failed to delete an object from the device because we were returned a NULL IPortableDevicePropVariantCollection interface pointer.", hr);
            else
            {
                PROPVARIANT pv = { 0 };
                PropVariantInit(&pv);

                // Initialize a PROPVARIANT structure with the object identifier string
                // that the user selected above. Notice we are allocating memory for the
                // PWSTR value.  This memory will be freed when PropVariantClear() is
                // called below.
                CPortableDeviceException* pEx = nullptr;
                pv.vt = VT_LPWSTR;
                pv.pwszVal = AtlAllocTaskWideString(pszObjectID);
                if (pv.pwszVal != NULL)
                {
                    // Add the object identifier to the objects-to-delete list
                    // (We are only deleting 1 in this example)
                    hr = pObjectsToDelete->Add(&pv);
                    if (FAILED(hr))
                    {
                        pEx = new CPortableDeviceException(L"DeleteContentFromDevice: Failed to delete an object from the device because we could no add the object identifier string to the IPortableDevicePropVariantCollection.", hr);
                    }
                    if (SUCCEEDED(hr))
                    {
                        // Attempt to delete the object from the device
                        hr = pContent->Delete(PORTABLE_DEVICE_DELETE_NO_RECURSION,  // Deleting with no recursion
                            pObjectsToDelete,                     // Object(s) to delete
                            NULL);                                // Object(s) that failed to delete (we are only deleting 1, so we can pass NULL here)
                        if (SUCCEEDED(hr))
                        {
                            // An S_OK return lets the caller know that the deletion was successful
                            if (hr == S_OK)
                            {
                                TRACE1("The object '%ws' was deleted from the device.\n", pszObjectID);
                            }

                            // An S_FALSE return lets the caller know that the deletion failed.
                            // The caller should check the returned IPortableDevicePropVariantCollection
                            // for a list of object identifiers that failed to be deleted.
                            else
                            {
                                TRACE1("The object '%ws' failed to be deleted from the device.\n", pszObjectID);
                                pEx = new CPortableDeviceException(L"DeleteContentFromDevice: The object failed to be deleted from the device.");
                            }
                        }
                        else
                        {
                            pEx = new CPortableDeviceException(L"DeleteContentFromDevice: Failed to delete an object from the device.", hr);
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    pEx = new CPortableDeviceException(L"DeleteContentFromDevice: Failed to delete an object from the device because we could no allocate memory for the object identifier string.", hr);
                }

                // Free any allocated values in the PROPVARIANT before exiting
                PropVariantClear(&pv);
                if (pEx != nullptr)
                    throw pEx;
            }
        }
    }
}

//#################################################

IMPLEMENT_DYNAMIC(CPortableDeviceException, CException)

CPortableDeviceException::CPortableDeviceException(LPCTSTR lpszErrorText, BOOL bAutoDelete) : CException(bAutoDelete)
{
    m_strErrorText = lpszErrorText;
    TRACE1("CPortableDeviceException: %s\n", m_strErrorText);
}

CPortableDeviceException::CPortableDeviceException(LPCTSTR lpszErrorText, HRESULT hr, BOOL bAutoDelete)
{
    m_strErrorText.Format(L"%s\nhr = 0x%lx: ", lpszErrorText, hr);
    TRACE1("CPortableDeviceException: %s\n", m_strErrorText);

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, hr, 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    m_strErrorText += messageBuffer;

    LocalFree(messageBuffer);
}

BOOL CPortableDeviceException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext) const
{
    ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));
    if (pnHelpContext != NULL)
        *pnHelpContext = 0;

    if (nMaxError == 0 || lpszError == NULL)
        return FALSE;

    LPTSTR dummy = lstrcpyn(lpszError, m_strErrorText, nMaxError);
    return TRUE;
}


