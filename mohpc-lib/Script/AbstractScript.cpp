#include <Shared.h>
#include <MOHPC/Script/AbstractScript.h>
#include <MOHPC/Managers/ScriptManager.h>

using namespace MOHPC;

const str& AbstractScript::Filename()
{
	return GetScriptManager()->GetString(m_Filename);
}

const_str AbstractScript::ConstFilename()
{
	return m_Filename;
}

bool AbstractScript::GetSourceAt(size_t sourcePos, str &sourceLine, int &column, int &line)
{
	if (!m_SourceBuffer || sourcePos >= m_SourceLength)
	{
		return false;
	}

	line = 1;
	column = 0;
	size_t posLine = 0;

	char *p = m_SourceBuffer;
	char old_token;

	for (size_t i = 0; i < sourcePos; i++, p++)
	{
		column++;

		if (*p == '\n')
		{
			line++;
			column = 0;
			posLine = i + 1;
		}
		else if (*p == '\0')
		{
			break;
		}
	}

	while (*p != '\0' && *p != '\n')
	{
		p++;
	}

	old_token = *p;
	*p = '\0';

	sourceLine = (m_SourceBuffer + posLine);

	*p = old_token;

	return true;
}

void AbstractScript::PrintSourcePos(sourceinfo_t *sourcePos, bool dev)
{
	int line;
	int column;
	str sourceLine;

	if (GetSourceAt(sourcePos->sourcePos, sourceLine, column, line))
	{
		PrintSourcePos(sourceLine, column, line, dev);
	}
	else
	{
		printf("file '%s', source pos %d line %d column %d:\n", Filename().c_str(), sourcePos->sourcePos, sourcePos->line, sourcePos->column);
	}
}

void AbstractScript::PrintSourcePos(size_t sourcePos, bool dev)
{
	int line;
	int column;
	str sourceLine;

	if (GetSourceAt(sourcePos, sourceLine, column, line))
	{
		PrintSourcePos(sourceLine, column, line, dev);
	}
	else
	{
		printf("file '%s', source pos %zd:\n", Filename().c_str(), sourcePos);
	}
}

void AbstractScript::PrintSourcePos(unsigned char *m_pCodePos, bool dev)
{
	if (!m_ProgToSource)
	{
		return;
	}

	sourceinfo_t *codePos = m_ProgToSource->findKeyValue(m_pCodePos);

	if (!codePos)
	{
		return;
	}

	PrintSourcePos(codePos, dev);
}

void AbstractScript::PrintSourcePos(str sourceLine, int column, int line, bool dev)
{
	int i;
	str markerLine = "";

	for (i = 0; i < column; i++)
	{
		markerLine.append(sourceLine[i]);
	}

	markerLine.append("^");

	printf("(%s, %d):\n%s\n%s\n", Filename().c_str(), line, sourceLine.c_str(), markerLine.c_str());
}

AbstractScript::AbstractScript()
{
	m_ProgToSource = NULL;
	m_SourceBuffer = NULL;
	m_SourceLength = 0;
}
