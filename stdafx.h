// stdafx.h : Includedatei für Standardsystem-Includedateien,
//  oder projektspezifische Includedateien, die häufig benutzt, aber
// in unregelmäßigen Abständen geändert werden.
//

#pragma once

// Ändern Sie folgende Definitionen für Plattformen, die älter als die unten angegebenen sind.
// Unter MSDN finden Sie die neuesten Informationen über die entsprechenden Werte für die unterschiedlichen Plattformen.
#ifndef WINVER				// Lassen Sie die Verwendung von Features spezifisch für Windows 95 und Windows NT 4 oder später zu.
#define WINVER 0x0600		// Ändern Sie den entsprechenden Wert, um auf Windows 98 und mindestens Windows 2000 abzuzielen.
#endif

#ifndef _WIN32_WINNT		// Lassen Sie die Verwendung von Features spezifisch für Windows NT 4 oder später zu.
#define _WIN32_WINNT 0x0600		// Ändern Sie den entsprechenden Wert, um auf Windows 98 und mindestens Windows 2000 abzuzielen.
#endif						

#ifndef _WIN32_WINDOWS		// Lassen Sie die Verwendung von Features spezifisch für Windows 98 oder später zu.
#define _WIN32_WINDOWS 0x0600 // Ändern Sie den entsprechenden Wert, um auf mindestens Windows Me abzuzielen.
#endif

#ifndef _WIN32_IE			// Lassen Sie die Verwendung von Features spezifisch für IE 4.0 oder später zu.
#define _WIN32_IE 0x0600	// Ändern Sie den entsprechenden Wert, um auf mindestens IE 5.0 abzuzielen.
#endif

#define WIN32_LEAN_AND_MEAN		// Selten verwendete Teile der Windows-Header nicht einbinden
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// einige CString-Konstruktoren sind explizit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Selten verwendete Teile der Windows-Header nicht einbinden
#endif

#include <afx.h>
#include <afxwin.h>         // MFC-Kern- und Standardkomponenten

// TODO: Verweisen Sie hier auf zusätzliche Header, die Ihr Programm erfordert
