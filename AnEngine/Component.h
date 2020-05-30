#pragma once
#pragma once
#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <mutex>

namespace AnEngine::Game
{
	class GameObject;


	/*
	 * Component ���ڴ�����ݣ��ҽ�������ݣ��ɶ��̶߳��͵��߳�д
	 */
	class Component
	{
		GameObject* m_entity;
	public:
		virtual ~Component() = default;
	};
}
#endif // __COMPONENT_H__

/*
 * һ��Component����ֻ������һ��Entity
 */
