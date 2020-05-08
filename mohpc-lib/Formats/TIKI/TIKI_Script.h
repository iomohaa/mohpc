#pragma once

#include <MOHPC/Asset.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Utilities/SharedPtr.h>
#include <string>

#define TOKENCOMMENT		(';')
#define TOKENCOMMENT2		('#')
#define TOKENEOL			('\n')
//#define TOKENNULL			('\0')
#define TOKENSPACE			(' ')
#define TOKENSPECIAL		('$')

#ifndef MAXTOKEN
#define	MAXTOKEN	256
#endif

#define MAXMACROS	48

namespace MOHPC
{
	typedef struct {
		char name[MAXTOKEN];
		char macro[MAXTOKEN];
	} tiki_macro_t;

	typedef struct {
		const char *mark_script_p;
		bool mark_tokenready;
		char mark_token[MAXTOKEN];
	} tiki_mark_t;

	typedef SafePtr<class TikiScript> TikiScriptPtr;

	class TikiScript : public Asset, public EnableSharedFromThis<TikiScript>
	{
		CLASS_BODY(TikiScript);

	protected:
		bool error;
		bool tokenready;
		SharedPtr<TikiScript> include;
		SharedPtr<TikiScript> parent;
		char filename[MAXTOKEN];
		const char *script_p;
		const char *end_p;
		tiki_macro_t macros[MAXMACROS];
		int nummacros;
		int line;
		char token[MAXTOKEN];
		bool releaseBuffer;
		bool allow_extended_comment;
		tiki_mark_t mark[64];
		int mark_pos;

	public:
		char *buffer;
		uintmax_t length;
		char path[256];
		SharedPtr<TikiScript> currentScript;

	protected:
		bool AtComment();
		bool AtExtendedComment();
		bool AtCommand();
		bool AtString(bool crossline);
		bool ProcessCommand(bool crossline);
		bool Completed();
		bool SafeCheckOverflow();
		void CheckOverflow();
		void Uninclude();
		const char *FindMacro(const char *macro);
		void AddMacro(const char *macro, const char *expansion);
		bool SkipToEOL();
		void SkipWhiteSpace(bool crossline);
		void SkipNonToken(bool crossline);
		bool CommentAvailable(bool crossline);
		void SkipExtendedComment();

	public:
		TikiScript();
		~TikiScript();

		virtual bool Load() override;
		void Close();
		const char *Filename();
		int GetLineNumber();
		void Reset();
		bool TokenAvailable(bool crossline);
		void UnGetToken();
		const char *GetToken(bool crossline);
		const char *GetLine(bool crossline);
		const char *GetAndIgnoreLine(bool crossline);
		const char *GetRaw();
		const char *GetString(bool crossline);
		bool GetSpecific(const char *string);
		int GetInteger(bool crossline);
		double GetDouble(bool crossline);
		float GetFloat(bool crossline);
		void GetVector(bool crossline, float *vec);
		int LinesInFile();
		void Parse(char *data, uintmax_t length, const char *name);
		bool LoadFile(const char *name);
		const char *Token();
		void MarkPos();
		void ReturnToMark();
		void ReplaceLineWithWhitespace(bool deleteFromStartOfLine);
		void Exclude();
		const char *GetParentToken();
		void SetCurrentScript(SharedPtr<TikiScript> Script);
		void SetAllowExtendedComment(bool bAllow);
	};

	class TikiSwitchKey
	{
	public:
		str sKeyName;
		str sKeyValue;
		TikiSwitchKey *pNextKey;

		TikiSwitchKey();
	};

	class TikiSwitcher {
		TikiSwitchKey *m_pSwitchKeys;

	private:
		TikiSwitchKey *GetSwitchKey(const char *);
		void RemoveUnwantedSwitchOptions(TikiScript *);

	public:
		TikiSwitcher();

		void AddSwitchKey(const char *, const char *);
		void ChangeSwitchKey(const char *, const char *);
		void AddOrChangeSwitchKey(const char *, const char *);
		str& GetSwitchKeyValue(const char *);
		void PrecompileTikiScript(TikiScript *);
	};
}
