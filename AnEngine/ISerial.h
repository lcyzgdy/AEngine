#pragma once
#ifndef __ISERIAL_H__
#define __ISERIAL_H__

namespace AnEngine::Game
{
	template<typename ..._ComponentType>
	class ISerial;

	/*
	 * Job �Ǵ�ִ�з����������� Component �е����ݣ��������� Behaviour �����ݡ�
	 * Job �����֣������Ǵ���ִ�е� Job����˲�����˴��������� Entity ��Component���������ܻ����½������ҿ���������
	 */
	template<>
	class ISerial<>
	{
	public:
		virtual void Execute(int index) = 0;
		virtual bool Check(int index) { return true; }
		size_t Length;
	};

	template<typename ..._ComponentType>
	class ISerial : public ISerial<>
	{
	};
}

#endif // !__ISERIAL_H__
