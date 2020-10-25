#include <Shared.h>
#include <MOHPC/Common/con_timer.h>
#include <MOHPC/Script/Archiver.h>
#include <MOHPC/Script/Archiver.h>

using namespace MOHPC;

con_timer::con_timer()
{
	m_time = 0;
	m_bDirty = false;
}

void con_timer::AddElement(Class *e, float time)
{
	Element element;

	element.obj = e;
	element.time = time;

	m_Elements.AddObject(element);

	if (time <= m_time)
	{
		SetDirty();
	}
}

void con_timer::RemoveElement(Class *e)
{
	for (intptr_t i = m_Elements.NumObjects(); i > 0; i--)
	{
		Element *index = &m_Elements.ObjectAt(i);

		if (index->obj == e)
		{
			m_Elements.RemoveObjectAt(i);
			return;
		}
	}
}

Class *con_timer::GetNextElement(float& foundtime)
{
	float best_time;
	intptr_t foundIndex;
	Class* result;

	foundIndex = 0;
	best_time = m_time;

	for (intptr_t i = m_Elements.NumObjects(); i > 0; i--)
	{
		if (m_Elements.ObjectAt(i).time <= best_time)
		{
			best_time = m_Elements.ObjectAt(i).time;
			foundIndex = i;
		}
	}

	if (foundIndex)
	{
		result = m_Elements.ObjectAt(foundIndex).obj;
		m_Elements.RemoveObjectAt(foundIndex);
		foundtime = best_time;
	}
	else
	{
		result = NULL;
		m_bDirty = false;
	}

	return result;
}

void con_timer::ArchiveElement(Archiver& arc, Element *e)
{
	/*
	arc.ArchiveObjectPointer(&e->obj);
	arc.ArchiveInteger(&e->inttime);
	*/
}

void con_timer::Archive(Archiver& arc)
{
	/*
	arc.ArchiveBool(&m_bDirty);
	arc.ArchiveInteger(&m_inttime);
	*/

	m_Elements.Archive(arc, con_timer::ArchiveElement);
}
