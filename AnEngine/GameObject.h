#pragma once
#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "onwind.h"
#include "Object.h"
#include "Transform.h"
#include <mutex>
#include <queue>
#include "ComponentData.h"
#include <set>
#include "Archetype.h"


namespace AnEngine::AssetsWrapper
{
	class AssetsDatabase;
}

namespace AnEngine::Game
{
	class ObjectBehaviour;

	// 一个确定的物体
	class GameObject : public Object
	{
		friend class Scene;
		friend class AssetsWrapper::AssetsDatabase;
		friend class std::shared_ptr<GameObject>;
		friend class std::unique_ptr<GameObject>;
		friend std::shared_ptr<GameObject> std::make_shared(std::string&&);

		template <class _Ty>
		friend void std::_Destroy_in_place<_Ty>(_Ty&) noexcept;
		template<typename _Ty, typename... _Types>
		friend void std::_Construct_in_place(_Ty& _Obj, _Types&&... _Args) noexcept(std::is_nothrow_constructible_v<_Ty, _Types...>);
		friend struct std::default_delete<GameObject>;

		std::mutex m_mutex;
		bool m_active;
		bool m_destoryed;
		uint64_t m_id; // 在Scene容器中的编号

		// Create的时候令destoryed为false，AddToScene的时候为m_id赋值，默认为-1。

	protected:
		// 当前物体的一些组件，比如渲染器、脚本等等。
		// Components of this object, e.g. script、renderer、rigidbody etc.
		std::vector<ObjectBehaviour*> m_behaviour;
		std::map<double, size_t> m_typeToId;

		std::string name;
		Archetype* m_archetype;
		std::vector<GameObject*> children;



		GameObject(const std::string& name);
		GameObject(std::string&& name);

		virtual ~GameObject();

	public:
		// GameObject(GameObject&& other);

		template<typename _Ty>
		_Ty* GetComponent()
		{
			return nullptr;
		}

		void AddBehaviour(ObjectBehaviour* component);

		template<typename T>
		T* GetBehaviour()
		{
			if constexpr (std::is_base_of<ObjectBehaviour, T>::value == false)
			{
				throw std::exception("Type is not derived ObjectBehaviour");
			}
			if (m_typeToId.find(typeid(T).hash_code()) != m_typeToId.end())
			{
				return m_behaviour[m_typeToId[typeid(T).hash_code()]];
			}
			return nullptr;
		}

		const std::vector<ObjectBehaviour*>& GetAllBehaviours() { return m_behaviour; }

		template<typename T>
		T* AddBehaviour()
		{
			if (m_typeToId.find(typeid(T).hash_code()) != m_typeToId.end()) throw std::exception("已经存在一个了");
			m_typeToId[typeid(T).hash_code()] = m_behaviour.size();
			m_behaviour.emplace_back(new T());
			return *m_behaviour.rbegin();
		}

		inline bool Active() { if (m_destoryed) throw std::exception("This object has already destoryed!");	return m_active; }
		void Active(bool b);

		inline bool Destoryed() { return m_destoryed; }

		__forceinline uint32_t Id() { return m_id; }

		static std::shared_ptr<GameObject> Create(const std::string& name);
		static std::shared_ptr<GameObject> Create(std::string&& name);
		static void Destory(GameObject* gameObject);
		static GameObject* Find(const std::string& name);
		static GameObject* Find(std::string&& name);
	};
}
#endif // !__GAMEOBJECT_H__



/*
 * 最终还是在 GameObject 里面添加了Component和Behaviour，如果彻底抛弃Behaviour则一些本来很简单的功能实现起来十分麻烦，
 * 而且之前写好的状态机也会报废。
 */
