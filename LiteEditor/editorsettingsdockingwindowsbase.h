//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// wxCrafter project file: editor_options_docking_windows.wxcp
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#ifndef _CODELITE_LITEEDITOR_EDITOR_OPTIONS_DOCKING_WINDOWS_BASE_CLASSES_H
#define _CODELITE_LITEEDITOR_EDITOR_OPTIONS_DOCKING_WINDOWS_BASE_CLASSES_H

#include <wx/settings.h>
#include <wx/xrc/xmlres.h>
#include <wx/xrc/xh_bmp.h>
#include <wx/panel.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/imaglist.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <wx/statline.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#if wxVERSION_NUMBER >= 2900
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/persist/bookctrl.h>
#include <wx/persist/treebook.h>
#endif

#ifdef WXC_FROM_DIP
#undef WXC_FROM_DIP
#endif
#if wxVERSION_NUMBER >= 3100
#define WXC_FROM_DIP(x) wxWindow::FromDIP(x, NULL)
#else
#define WXC_FROM_DIP(x) x
#endif


class EditorSettingsDockingWindowsBase : public wxPanel
{
protected:
    wxNotebook* m_notebook10;
    wxPanel* m_panelTabs;
    wxStaticText* m_staticText46;
    wxChoice* m_choiceTabStyle;
    wxStaticText* m_staticTextTabHeight;
    wxChoice* m_choiceTabHeight;
    wxStaticText* m_staticText38;
    wxChoice* m_choiceWorkspaceTabsOrientation;
    wxStaticText* m_staticText42;
    wxChoice* m_choiceOutputTabsOrientation;
    wxStaticLine* m_staticLine61;
    wxCheckBox* m_checkBoxShowXButton;
    wxCheckBox* m_checkBoxEditorTabsFollowsTheme;
    wxCheckBox* m_checkBoxUseDarkTabTheme;
    wxCheckBox* m_checkBoxMouseScrollSwitchTabs;
    wxPanel* m_panel12;
    wxRadioBox* m_radioBoxHint;
    wxCheckBox* m_checkBoxHideCaptions;
    wxCheckBox* m_checkBoxEnsureCaptionsVisible;
    wxPanel* m_panel14;
    wxCheckBox* m_checkBoxFindBarAtBottom;
    wxCheckBox* m_checkBoxDontFoldSearchResults;
    wxPanel* m_panel16;
    wxCheckBox* m_checkBoxHideOutputPaneOnClick;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfBuild;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfSearch;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfReplace;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfReferences;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfOutput;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfTrace;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfTasks;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfBuildQ;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfCppCheck;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfSvn;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfCscope;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfGit;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfDebug;
    wxCheckBox* m_checkBoxHideOutputPaneNotIfMemCheck;
    wxCheckBox* m_checkBoxShowDebugOnRun;

protected:
    virtual void OnEnsureCaptionsVisibleUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnHideOutputPaneNotIfDebugUI(wxUpdateUIEvent& event) { event.Skip(); }

public:
    wxStaticText* GetStaticText46() { return m_staticText46; }
    wxChoice* GetChoiceTabStyle() { return m_choiceTabStyle; }
    wxStaticText* GetStaticTextTabHeight() { return m_staticTextTabHeight; }
    wxChoice* GetChoiceTabHeight() { return m_choiceTabHeight; }
    wxStaticText* GetStaticText38() { return m_staticText38; }
    wxChoice* GetChoiceWorkspaceTabsOrientation() { return m_choiceWorkspaceTabsOrientation; }
    wxStaticText* GetStaticText42() { return m_staticText42; }
    wxChoice* GetChoiceOutputTabsOrientation() { return m_choiceOutputTabsOrientation; }
    wxStaticLine* GetStaticLine61() { return m_staticLine61; }
    wxCheckBox* GetCheckBoxShowXButton() { return m_checkBoxShowXButton; }
    wxCheckBox* GetCheckBoxEditorTabsFollowsTheme() { return m_checkBoxEditorTabsFollowsTheme; }
    wxCheckBox* GetCheckBoxUseDarkTabTheme() { return m_checkBoxUseDarkTabTheme; }
    wxCheckBox* GetCheckBoxMouseScrollSwitchTabs() { return m_checkBoxMouseScrollSwitchTabs; }
    wxPanel* GetPanelTabs() { return m_panelTabs; }
    wxRadioBox* GetRadioBoxHint() { return m_radioBoxHint; }
    wxCheckBox* GetCheckBoxHideCaptions() { return m_checkBoxHideCaptions; }
    wxCheckBox* GetCheckBoxEnsureCaptionsVisible() { return m_checkBoxEnsureCaptionsVisible; }
    wxPanel* GetPanel12() { return m_panel12; }
    wxCheckBox* GetCheckBoxFindBarAtBottom() { return m_checkBoxFindBarAtBottom; }
    wxCheckBox* GetCheckBoxDontFoldSearchResults() { return m_checkBoxDontFoldSearchResults; }
    wxPanel* GetPanel14() { return m_panel14; }
    wxCheckBox* GetCheckBoxHideOutputPaneOnClick() { return m_checkBoxHideOutputPaneOnClick; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfBuild() { return m_checkBoxHideOutputPaneNotIfBuild; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfSearch() { return m_checkBoxHideOutputPaneNotIfSearch; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfReplace() { return m_checkBoxHideOutputPaneNotIfReplace; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfReferences() { return m_checkBoxHideOutputPaneNotIfReferences; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfOutput() { return m_checkBoxHideOutputPaneNotIfOutput; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfTrace() { return m_checkBoxHideOutputPaneNotIfTrace; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfTasks() { return m_checkBoxHideOutputPaneNotIfTasks; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfBuildQ() { return m_checkBoxHideOutputPaneNotIfBuildQ; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfCppCheck() { return m_checkBoxHideOutputPaneNotIfCppCheck; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfSvn() { return m_checkBoxHideOutputPaneNotIfSvn; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfCscope() { return m_checkBoxHideOutputPaneNotIfCscope; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfGit() { return m_checkBoxHideOutputPaneNotIfGit; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfDebug() { return m_checkBoxHideOutputPaneNotIfDebug; }
    wxCheckBox* GetCheckBoxHideOutputPaneNotIfMemCheck() { return m_checkBoxHideOutputPaneNotIfMemCheck; }
    wxCheckBox* GetCheckBoxShowDebugOnRun() { return m_checkBoxShowDebugOnRun; }
    wxPanel* GetPanel16() { return m_panel16; }
    wxNotebook* GetNotebook10() { return m_notebook10; }
    EditorSettingsDockingWindowsBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
    virtual ~EditorSettingsDockingWindowsBase();
};

#endif
