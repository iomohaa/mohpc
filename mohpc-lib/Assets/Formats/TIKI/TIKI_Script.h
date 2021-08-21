#pragma once

#include <MOHPC/Assets/Asset.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Utility/SharedPtr.h>
#include <string>

namespace MOHPC
{
	static constexpr char TOKENCOMMENT(';');
	static constexpr char TOKENCOMMENT2('#');
	static constexpr char TOKENEOL('\n');
	static constexpr char TOKENNULL('\0');
	static constexpr char TOKENSPACE(' ');
	static constexpr char TOKENSPECIAL('$');
	static constexpr unsigned int MAXTOKEN = 256;

	static constexpr unsigned int MAXMACROS = 48;

	typedef struct {
		char name[MAXTOKEN];
		char macro[MAXTOKEN];
	} tiki_macro_t;

	typedef struct {
		const char *mark_script_p;
		bool mark_tokenready;
		char mark_token[MAXTOKEN];
	} tiki_mark_t;

	class TikiScript : public Asset, public EnableSharedFromThis<TikiScript>
	{
		MOHPC_ASSET_OBJECT_DECLARATION(TikiScript);

	public:
		TikiScript(const SharedPtr<AssetManager>& assetManagerPtr, const fs::path& fileNameRef);
		~TikiScript();

		void Load();
		void Close();
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
		void LoadFile(const IFilePtr& file);
		const char *Token();
		void MarkPos();
		void ReturnToMark();
		void ReplaceLineWithWhitespace(bool deleteFromStartOfLine);
		void Exclude();
		const char *GetParentToken();
		void SetCurrentScript(SharedPtr<TikiScript> Script);
		const SharedPtr<MOHPC::TikiScript>& getCurrentScript() const;
		void SetAllowExtendedComment(bool bAllow);
		void setPath(const char* newPath);
		const std::string& getPath() const;

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
		const char* FindMacro(const char* macro);
		void AddMacro(const char* macro, const char* expansion);
		bool SkipToEOL();
		void SkipWhiteSpace(bool crossline);
		void SkipNonToken(bool crossline);
		bool CommentAvailable(bool crossline);
		void SkipExtendedComment();

	private:
		SharedPtr<AssetManager> assetManager;
		char* buffer;
		SharedPtr<TikiScript> currentScript;
		SharedPtr<TikiScript> include;
		SharedPtr<TikiScript> parent;
		const char* script_p;
		const char* end_p;
		uintmax_t length;
		tiki_macro_t macros[MAXMACROS];
		tiki_mark_t mark[64];
		std::string path;
		char token[MAXTOKEN];
		int nummacros;
		int line;
		int mark_pos;
		bool releaseBuffer;
		bool allow_extended_comment;
		bool error;
		bool tokenready;
	};
	using TikiScriptPtr = SharedPtr<TikiScript>;

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

	class TikiScriptReader : public AssetReader
	{
		MOHPC_ASSET_OBJECT_DECLARATION(TikiScriptReader);

	public:
		using AssetType = TikiScript;

	public:
		AssetPtr read(const IFilePtr& file) override;
	};
}
