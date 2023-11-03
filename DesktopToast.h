#pragma once
#include <windows.ui.notifications.h>
#include <wrl.h>
#include <wrl\wrappers\corewrappers.h>

using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

class CDesktopToast
{
public:
	static HRESULT SetImageSrc(
		_In_ PCWSTR imagePath,
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* toastXml
	);
	static HRESULT SetTextValues(
		_In_reads_(textValuesCount) const PCWSTR* textValues,
		_In_ UINT32 textValuesCount,
		_Inout_ ABI::Windows::Data::Xml::Dom::IXmlDocument* toastXml
	);
	static HRESULT SetNodeValueString(
		_In_ HSTRING onputString,
		_Inout_ ABI::Windows::Data::Xml::Dom::IXmlNode* node,
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml
	);
	static HRESULT ShowToast(
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml
	);
	static HRESULT ClearToasts();
	static HRESULT SendBasicToast(PCWSTR message);
};

