#if HAS_LIBCLANG

#include <wx/app.h>
#include "clang_pch_maker_thread.h"
#include <wx/thread.h>
#include <wx/stdpaths.h>
#include "event_notifier.h"
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include "y.tab.h"
#include "cpp_scanner.h"
#include "file_logger.h"
#include "globals.h"
#include "procutils.h"
#include "fileextmanager.h"
#include <wx/xrc/xmlres.h>
#include "clang_utils.h"

#define cstr(x) x.mb_str(wxConvUTF8).data()

const wxEventType wxEVT_CLANG_PCH_CACHE_STARTED = XRCID("clang_pch_cache_started");
const wxEventType wxEVT_CLANG_PCH_CACHE_ENDED   = XRCID("clang_pch_cache_ended");
const wxEventType wxEVT_CLANG_PCH_CACHE_CLEARED = XRCID("clang_pch_cache_cleared");
const wxEventType wxEVT_CLANG_TU_CREATE_ERROR   = XRCID("clang_pch_create_error");
extern const wxEventType wxEVT_UPDATE_STATUS_BAR;

ClangWorkerThread::ClangWorkerThread()
{
	clang_toggleCrashRecovery(1);
}

ClangWorkerThread::~ClangWorkerThread()
{
}

void ClangWorkerThread::ProcessRequest(ThreadRequest* request)
{
	// Send start event
	PostEvent(wxEVT_CLANG_PCH_CACHE_STARTED);

	ClangThreadRequest *task = dynamic_cast<ClangThreadRequest*>( request );
	if( !task ) {
		PostEvent(wxEVT_CLANG_PCH_CACHE_ENDED);
		return;
	}

	{
		// A bit of optimization
		wxCriticalSectionLocker locker(m_criticalSection);
		if(task->GetContext() == CTX_CachePCH && m_cache.Contains(task->GetFileName())) {
			// Nothing to be done here
			PostEvent(wxEVT_CLANG_PCH_CACHE_ENDED);
			return;
		}
	}

	CL_DEBUG(wxT("==========> [ ClangPchMakerThread ] ProcessRequest started: %s"), task->GetFileName().c_str());
	std::string c_dirtyBuffer = cstr(task->GetDirtyBuffer());
	std::string c_filename    = cstr(task->GetFileName());

	CL_DEBUG(wxT("ClangWorkerThread:: processing request %d"), (int)task->GetContext());

	CXUnsavedFile unsavedFile = { c_filename.c_str(), c_dirtyBuffer.c_str(), c_dirtyBuffer.length() };

	ClangCacheEntry cacheEntry = findEntry(task->GetFileName());
	CXTranslationUnit TU = cacheEntry.TU;
	CL_DEBUG(wxT("ClangWorkerThread:: found cached TU: %p"), (void*)TU);
	bool reparseRequired = true;

	if(!TU) {
		TU = DoCreateTU(task->GetIndex(), task, true);
		reparseRequired = false;
	}

	if(!TU) {
		DoSetStatusMsg(wxT("Ready"));
		CL_DEBUG(wxT("Failed to parse Translation UNIT..."));
		PostEvent(wxEVT_CLANG_TU_CREATE_ERROR);
		return;
	}

	if(reparseRequired && task->GetContext() == ::CTX_ReparseTU) {
		DoSetStatusMsg(wxString::Format(wxT("clang: re-parsing file %s..."), task->GetFileName().c_str()));

		// We need to reparse the TU
		CL_DEBUG(wxT("Calling clang_reparseTranslationUnit... [CTX_ReparseTU]"));
		if(clang_reparseTranslationUnit(TU, 0, NULL, clang_defaultReparseOptions(TU)) == 0) {
			CL_DEBUG(wxT("Calling clang_reparseTranslationUnit... done [CTX_ReparseTU]"));

		} else {

			CL_DEBUG(wxT("An error occured during reparsing of the TU for file %s. TU: %p"), task->GetFileName().c_str(), (void*)TU);

			// The only thing that left to be done here, is to dispose the TU
			clang_disposeTranslationUnit(TU);
			PostEvent(wxEVT_CLANG_TU_CREATE_ERROR);

			return;
		}
	}

	// Construct a cache-returner class
	// which makes sure that the TU is cached
	// when we leave the current scope
	CacheReturner cr(this, task->GetFileName(), task->GetPchFile(), TU);

	DoSetStatusMsg(wxT("Ready"));

	// Prepare the 'End' event
	wxCommandEvent eEnd(wxEVT_CLANG_PCH_CACHE_ENDED);
	ClangThreadReply *reply = new ClangThreadReply;
	reply->context    = task->GetContext();
	reply->filterWord = task->GetFilterWord();
	reply->filename   = task->GetFileName().c_str();
	reply->results    = NULL;

	if( task->GetContext() == CTX_CodeCompletion ||
	    task->GetContext() == CTX_WordCompletion ||
	    task->GetContext() == CTX_Calltip) {
		CL_DEBUG(wxT("Calling clang_codeCompleteAt..."));
		CL_DEBUG(wxT("Location: %s:%u:%u"), task->GetFileName().c_str(), task->GetLine(), task->GetColumn());
		reply->results = clang_codeCompleteAt(TU,
		                                      cstr(task->GetFileName()),
		                                      task->GetLine(),
		                                      task->GetColumn(),
		                                      &unsavedFile,
		                                      1,
		                                      clang_defaultCodeCompleteOptions());

		CL_DEBUG(wxT("Calling clang_codeCompleteAt... done"));
		wxString displayTip;
		bool hasErrors(false);
		if(reply->results) {
			unsigned maxErrorToDisplay = 10;
			std::set<wxString> errorMessages;
			unsigned errorCount = clang_codeCompleteGetNumDiagnostics(reply->results);
			// Collect all errors / fatal errors and report them back to user
			for(unsigned i=0; i<errorCount; i++) {
				CXDiagnostic         diag     = clang_codeCompleteGetDiagnostic(reply->results, i);
				CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diag);
				if(!hasErrors) {
					hasErrors = (severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal);
				}

				if(severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal || severity == CXDiagnostic_Note) {
					CXString diagStr      = clang_getDiagnosticSpelling(diag);
					wxString wxDiagString = wxString(clang_getCString(diagStr), wxConvUTF8);

					// Collect up to 10 error messages
					// and dont collect the same error twice
					if(errorMessages.find(wxDiagString) == errorMessages.end() && errorMessages.size() <= maxErrorToDisplay) {
						errorMessages.insert(wxDiagString);
						displayTip << wxDiagString.c_str() << wxT("\n");
					}

					clang_disposeString(diagStr);
				}
				clang_disposeDiagnostic(diag);
			}

			CL_DEBUG(wxT("Found %u matches"), reply->results->NumResults);
			ClangUtils::printCompletionDiagnostics(reply->results);
		}

		if(!displayTip.IsEmpty() && hasErrors) {

			// Send back the error messages
			reply->errorMessage << wxT("Code Completion Error\n@@LINE@@\n") << displayTip;

			// Free the results
			clang_disposeCodeCompleteResults(reply->results);
			reply->results = NULL;
			reply->errorMessage.RemoveLast();
		}

	} else if(task->GetContext() == CTX_GotoDecl || task->GetContext() == CTX_GotoImpl) {
		DoGotoDefinition(TU, task, reply);

	}

	eEnd.SetClientData(reply);
	EventNotifier::Get()->AddPendingEvent(eEnd);
}

ClangCacheEntry ClangWorkerThread::findEntry(const wxString& filename)
{
	wxCriticalSectionLocker locker(m_criticalSection);
	ClangCacheEntry entry = m_cache.GetPCH(filename);
	return entry;
}

void ClangWorkerThread::DoCacheResult(CXTranslationUnit TU, const wxString& filename, const wxString& pch)
{
	wxCriticalSectionLocker locker(m_criticalSection);
	ClangCacheEntry cacheEntry;
	cacheEntry.TU = TU;
	cacheEntry.fileTU = pch;
	cacheEntry.sourceFile = filename;
	m_cache.AddPCH(cacheEntry);

	CL_DEBUG(wxT("caching Translation Unit file: %s, %p"), filename.c_str(), (void*)TU);
	CL_DEBUG(wxT(" ==========> [ ClangPchMakerThread ] PCH creation ended successfully <=============="));
}

void ClangWorkerThread::ClearCache()
{
	{
		wxCriticalSectionLocker locker(m_criticalSection);
		m_cache.Clear();
	}

	this->DoSetStatusMsg(wxT("clang: cache cleared"));

	// Notify about cache clear
	wxCommandEvent e(wxEVT_CLANG_PCH_CACHE_CLEARED);
	EventNotifier::Get()->AddPendingEvent(e);
}

bool ClangWorkerThread::IsCacheEmpty()
{
	wxCriticalSectionLocker locker(m_criticalSection);
	return m_cache.IsEmpty();
}

char** ClangWorkerThread::MakeCommandLine(ClangThreadRequest* req, int& argc, FileExtManager::FileType fileType)
{
	bool isHeader = !(fileType == FileExtManager::TypeSourceC || fileType == FileExtManager::TypeSourceCpp);
	wxArrayString tokens;
	const wxArrayString &command = req->GetCompilationArgs();
	tokens.insert(tokens.end(), command.begin(), command.end());
	if(isHeader) {
		tokens.Add(wxT("-x"));
		tokens.Add(wxT("c++-header"));
	}

	if(fileType == FileExtManager::TypeSourceC) {
		tokens.Add(wxT("-std=gnu++98"));
	}

#ifdef __WXMSW__
	tokens.Add(wxT("-std=c++0x"));
#endif
	tokens.Add(wxT("-w"));
	tokens.Add(wxT("-ferror-limit=1000"));
	tokens.Add(wxT("-nobuiltininc"));

#ifdef _MSC_VER
	tokens.Add(wxT("-fms-extensions"));
	tokens.Add(wxT("-fdelayed-template-parsing"));
#endif

//    static bool gotPCH = false;
//    if(!gotPCH) {
//        int nArgc(0);
//        wxArrayString pchTokens;
//        pchTokens.insert(pchTokens.end(), tokens.begin(), tokens.end());
//        pchTokens.Add(wxT("-x"));
//        pchTokens.Add(wxT("c++-header"));
//        char **pArgv = ClangUtils::MakeArgv(pchTokens, nArgc);
//        CXTranslationUnit PCH = clang_parseTranslationUnit(req->GetIndex(), "/home/eran/mypch.h", pArgv, nArgc,
//                                NULL,
//                                0,
//                                CXTranslationUnit_Incomplete);
//
//        ClangUtils::FreeArgv(pArgv, nArgc);
//
//        if(PCH) {
//            clang_saveTranslationUnit(PCH, "/home/eran/mypch.pch", clang_defaultSaveOptions(PCH));
//            clang_disposeTranslationUnit(PCH);
//
//            tokens.Add(wxT("-include-pch"));
//            tokens.Add(wxT("/home/eran/mypch.pch"));
//        }
//        gotPCH = true;
//    }

	char **argv = ClangUtils::MakeArgv(tokens, argc);
	return argv;
}

void ClangWorkerThread::DoSetStatusMsg(const wxString& msg)
{
	wxCommandEvent e(wxEVT_UPDATE_STATUS_BAR);
	e.SetString(msg.c_str());
	e.SetInt(0);
	e.SetId(10);
	EventNotifier::Get()->TopFrame()->GetEventHandler()->AddPendingEvent(e);
}

void ClangWorkerThread::DoGotoDefinition(CXTranslationUnit& TU, ClangThreadRequest* request, ClangThreadReply* reply)
{
	// Test to see if we are pointing a function
	CXCursor cur;
	if(ClangUtils::GetCursorAt(TU, request->GetFileName(), request->GetLine(), request->GetColumn(), cur)) {

		if(request->GetContext() == CTX_GotoImpl && !clang_isCursorDefinition(cur)) {
			cur = clang_getCursorDefinition(cur);
		}

		ClangUtils::GetCursorLocation(cur, reply->filename, reply->line, reply->col);
	}
}

void ClangWorkerThread::PostEvent(int type)
{
	wxCommandEvent e(type);
	e.SetClientData(NULL);
	EventNotifier::Get()->AddPendingEvent(e);
}

CXTranslationUnit ClangWorkerThread::DoCreateTU(CXIndex index, ClangThreadRequest* task, bool reparse)
{
	wxFileName fn(task->GetFileName());
	DoSetStatusMsg(wxString::Format(wxT("clang: parsing file %s..."), fn.GetFullName().c_str()));

	FileExtManager::FileType type = FileExtManager::GetType(task->GetFileName());
	int argc(0);
	char **argv = MakeCommandLine(task, argc, type);

	for(int i=0; i<argc; i++) {
		CL_DEBUG(wxT("Command Line Argument: %s"), wxString(argv[i], wxConvUTF8).c_str());
	}

	std::string c_filename = task->GetFileName().mb_str(wxConvUTF8).data();
	CL_DEBUG(wxT("Calling clang_parseTranslationUnit..."));

	// First time, need to create it
	unsigned flags;
	if(reparse) {
		flags = CXTranslationUnit_CXXPrecompiledPreamble
		        | CXTranslationUnit_CacheCompletionResults
		        | CXTranslationUnit_PrecompiledPreamble
		        | CXTranslationUnit_Incomplete
		        | CXTranslationUnit_DetailedPreprocessingRecord
		        | CXTranslationUnit_CXXChainedPCH
		        | CXTranslationUnit_SkipFunctionBodies;
	} else {
		flags = CXTranslationUnit_Incomplete
		        | CXTranslationUnit_SkipFunctionBodies
		        | CXTranslationUnit_DetailedPreprocessingRecord;
	}

	CXTranslationUnit TU = clang_parseTranslationUnit(index,
	                       c_filename.c_str(),
	                       argv,
	                       argc,
	                       NULL, 0, flags);

	CL_DEBUG(wxT("Calling clang_parseTranslationUnit... done"));
	ClangUtils::FreeArgv(argv, argc);

	if(TU && reparse) {
		CL_DEBUG(wxT("Calling clang_reparseTranslationUnit..."));
		if(clang_reparseTranslationUnit(TU, 0, NULL, clang_defaultReparseOptions(TU)) == 0) {
			CL_DEBUG(wxT("Calling clang_reparseTranslationUnit... done"));
			return TU;

		} else {
			CL_DEBUG(wxT("An error occured during reparsing of the TU for file %s. TU: %p"), task->GetFileName().c_str(), (void*)TU);

			// The only thing that left to be done here, is to dispose the TU
			clang_disposeTranslationUnit(TU);
			PostEvent(wxEVT_CLANG_TU_CREATE_ERROR);
			return NULL;
		}
	}
	return TU;
}

#endif // HAS_LIBCLANG
