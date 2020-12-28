========================================================================
    STATISCHE BIBLIOTHEK : PPLib-Projektübersicht
========================================================================

Der Anwendungs-Assistent hat das PPLib-Bibliothekprojekt erstellt. 
Diese Datei enthält eine Übersicht des Inhalts der Dateien in der
 PPLib-Anwendung.


PPLib.vcproj
    Dies ist die Hauptprojektdatei für VC++-Projekte, die vom Anwendungs-Assistenten erstellt wird. 
    Sie enthält Informationen über die Version von Visual C++, mit der 
    die Datei generiert wurde, über die Plattformen, Konfigurationen und Projektfeatures,
    die mit dem Anwendungs-Assistenten ausgewählt wurden.


/////////////////////////////////////////////////////////////////////////////

StdAfx.h, StdAfx.cpp
    Mit diesen Dateien werden vorkompilierte Headerdateien (PCH)
    mit der Bezeichnung PPLib.pch und eine vorkompilierte Typdatei mit der Bezeichnung StdAfx.obj erstellt.

/////////////////////////////////////////////////////////////////////////////
Die Compiler- und Linkerschalter wurden geändert, um MFC zu unterstützen. Wenn Sie den
MFC-Klassenassistenten für dieses Projekt verwenden, müssen Sie mehrere Dateien zum Projekt 
hinzufügen, einschl. der Dateien "resource.h", "PPLib.rc" und "PPLib.h", die die 
Datei resource.h enthält. Wenn Sie einen RC-Datei zu einer statischen Bibliothek hinzufügen, treten möglicherweise Probleme auf, 
da in einer DLL- oder EXE-Datei nur eine RC-Datei vorhanden sein 
darf. Sie können dieses Problem umgehen, indem Sie die RC-Datei der 
Bibliothek in die RC-Datei des übergeordneten Projekts einbeziehen.

/////////////////////////////////////////////////////////////////////////////
Weitere Hinweise:

Der Anwendungs-Assistent verwendet "TODO:"-Kommentare, um Teile des Quellcodes anzuzeigen, die hinzugefügt oder angepasst werden müssen.

/////////////////////////////////////////////////////////////////////////////
