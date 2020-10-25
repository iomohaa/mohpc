#pragma once

#include "../Class.h"
#include "../Common/Container.h"

namespace MOHPC
{
	class con_timer : public Class
	{
	public:
		class Element
		{
		public:
			Class* obj;
			float time;
		};

	private:
		Container<con_timer::Element> m_Elements;
		bool m_bDirty;
		float m_time;

	public:
		con_timer();

		void AddElement(Class *e, float time);
		void RemoveElement(Class *e);

		Class* GetNextElement(float& foundTime);

		void SetDirty() { m_bDirty = true; };
		bool IsDirty() { return m_bDirty; };
		void SetTime(float time) { m_time = time; };

		static void ArchiveElement(Archiver& arc, Element *e);
		virtual void Archive(Archiver& arc);
	};
}
