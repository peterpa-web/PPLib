#include "stdafx.h"
#include "DesktopNotificationManagerCompat.h"

#include "DesktopToast.h"

#define RETURN_IF_FAILED(hr) do { HRESULT _hrTemp = hr; if (FAILED(_hrTemp)) { return _hrTemp; } } while (false)

// Set the value of the "src" attribute of the "image" node
HRESULT CDesktopToast::SetImageSrc(PCWSTR imagePath, IXmlDocument* toastXml)
{
	wchar_t imageSrcUri[MAX_PATH];
	DWORD size = ARRAYSIZE(imageSrcUri);

	RETURN_IF_FAILED(::UrlCreateFromPath(imagePath, imageSrcUri, &size, 0));

	ComPtr<IXmlNodeList> nodeList;
	RETURN_IF_FAILED(toastXml->GetElementsByTagName(HStringReference(L"image").Get(), &nodeList));

	ComPtr<IXmlNode> imageNode;
	RETURN_IF_FAILED(nodeList->Item(0, &imageNode));

	ComPtr<IXmlNamedNodeMap> attributes;

	RETURN_IF_FAILED(imageNode->get_Attributes(&attributes));

	ComPtr<IXmlNode> srcAttribute;
	RETURN_IF_FAILED(attributes->GetNamedItem(HStringReference(L"src").Get(), &srcAttribute));

	return SetNodeValueString(HStringReference(imageSrcUri).Get(), srcAttribute.Get(), toastXml);
}

// Set the values of each of the text nodes
HRESULT CDesktopToast::SetTextValues(const PCWSTR* textValues, UINT32 textValuesCount, IXmlDocument* toastXml)
{
	ComPtr<IXmlNodeList> nodeList;
	RETURN_IF_FAILED(toastXml->GetElementsByTagName(HStringReference(L"text").Get(), &nodeList));

	UINT32 nodeListLength;
	RETURN_IF_FAILED(nodeList->get_Length(&nodeListLength));

	// If a template was chosen with fewer text elements, also change the amount of strings
	// passed to this method.
	RETURN_IF_FAILED(textValuesCount <= nodeListLength ? S_OK : E_INVALIDARG);

	for (UINT32 i = 0; i < textValuesCount; i++)
	{
		ComPtr<IXmlNode> textNode;
		RETURN_IF_FAILED(nodeList->Item(i, &textNode));

		RETURN_IF_FAILED(SetNodeValueString(HStringReference(textValues[i]).Get(), textNode.Get(), toastXml));
	}

	return S_OK;
}

HRESULT CDesktopToast::SetNodeValueString(HSTRING inputString, IXmlNode* node, IXmlDocument* xml)
{
	ComPtr<IXmlText> inputText;
	RETURN_IF_FAILED(xml->CreateTextNode(inputString, &inputText));

	ComPtr<IXmlNode> inputTextNode;
	RETURN_IF_FAILED(inputText.As(&inputTextNode));

	ComPtr<IXmlNode> appendedChild;
	return node->AppendChild(inputTextNode.Get(), &appendedChild);
}

// Create and display the toast
HRESULT CDesktopToast::ShowToast(IXmlDocument* xml)
{
	// Create the notifier
	// Classic Win32 apps MUST use the compat method to create the notifier
	ComPtr<IToastNotifier> notifier;
	RETURN_IF_FAILED(DesktopNotificationManagerCompat::CreateToastNotifier(&notifier));

	// And create the notification itself
	ComPtr<IToastNotification> toast;
	RETURN_IF_FAILED(DesktopNotificationManagerCompat::CreateToastNotification(xml, &toast));

	// And show it!
	return notifier->Show(toast.Get());
}

// Clear all toasts
HRESULT CDesktopToast::ClearToasts()
{
	// Get the history object
	// Classic Win32 apps MUST use the compat method to obtain history
	std::unique_ptr<DesktopNotificationHistoryCompat> history;
	RETURN_IF_FAILED(DesktopNotificationManagerCompat::get_History(&history));

	// And clear the toasts
	return history->Clear();
}

HRESULT CDesktopToast::SendBasicToast(PCWSTR message)
{
	ComPtr<IXmlDocument> doc;
	RETURN_IF_FAILED(DesktopNotificationManagerCompat::CreateXmlDocumentFromString(
		LR"(<toast>
    <visual>
        <binding template="ToastGeneric">
            <text></text>
        </binding>
    </visual>
</toast>)", &doc));

	PCWSTR textValues[] = {
		message
	};
	RETURN_IF_FAILED(SetTextValues(textValues, ARRAYSIZE(textValues), doc.Get()));

	return ShowToast(doc.Get());
}
