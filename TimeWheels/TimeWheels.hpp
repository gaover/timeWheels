#pragma once

#include <iostream>
#include <vector>
#include <functional>

namespace tws
{

#define TIMER_ACCURACY 50

	enum ETimerLevel
	{
		eSecondLvl,
		eMiniteLvl,
		eHourLvl,

		TimerLvlNUM,
	};

	static std::uint32_t sg_TimerLimit[TimerLvlNUM] = { 1000, 60 * 1000 + 1000, 60 * 60 * 1000 + 60 * 1000 + 1000 };
	static std::uint32_t sg_TimerDiffLimit[TimerLvlNUM] = { 0, 1000, 61 * 1000 };
	static std::uint32_t sg_TimerRateLimit[TimerLvlNUM] = { 50, 1000, 60 * 1000 };
	static std::uint32_t sg_TimerSlotLimit[TimerLvlNUM] = { 20, 60, 60 };

#define TIMER_MAX_ACCURACY sg_TimerLimit[eHourLvl]

	class Wheelement
	{
	public:
		Wheelement(std::int64_t id, std::int32_t timerID, std::int32_t interval)
			:m_i64ObjectID(id)
			, m_u32TimerID(timerID)
			, m_u32Interval(interval)
			, m_bIsOver(false)
		{

		}

		inline std::int64_t GetID() {
			return m_i64ObjectID;
		}
		inline std::uint32_t GetTimerID() {
			return m_u32TimerID;
		}
		inline std::uint32_t GetInterval() {
			return m_u32Interval;
		}
		inline bool IsOver() {
			return m_bIsOver;
		}
		inline void SetOver() {
			m_bIsOver = true;
		}
	private:
		std::int64_t	m_i64ObjectID;
		std::uint32_t	m_u32TimerID;
		std::uint32_t	m_u32Interval;
		bool			m_bIsOver;
	};

	typedef std::vector<Wheelement*> TimerSlot;
	typedef std::function<bool(Wheelement*)> TimerFunc;

	// 最高1小时定时处理1000*60*60
	class TimeWheels
	{
	public:
		TimeWheels()
		{
			for (int i = eSecondLvl; i != TimerLvlNUM; ++i)
			{
				m_kTimerData[i].resize(sg_TimerSlotLimit[i]);
				m_SlotIndex[i] = 0;
			}

			m_pkDispatchFunc = nullptr;
			m_i32LeftMs = 0;
		}

		void SetDispatchFunc(TimerFunc* pkFunc)
		{
			m_pkDispatchFunc = pkFunc;
		}

		bool Update(int iDeltaTime)
		{
			m_i32LeftMs += iDeltaTime;

			if (m_i32LeftMs < TIMER_ACCURACY)
			{
				return false;
			}

			int iStep = m_i32LeftMs / TIMER_ACCURACY;
			m_i32LeftMs %= TIMER_ACCURACY;

			auto& secondIndex = m_SlotIndex[eSecondLvl];

			for (int i = 0; i != iStep; i++)
			{
				TimerSlot& tasks = m_kTimerData[eSecondLvl][secondIndex++];

				for (auto& e : tasks)
				{
					if (e->IsOver())
					{
						delete e;
						continue;
					}


					if ((*m_pkDispatchFunc)(e))
					{
						AddNewTimer(e, false);
						continue;
					}

					delete e;
				}

				tasks.clear();

				if (secondIndex >= sg_TimerSlotLimit[eSecondLvl])
				{
					secondIndex = 0;
					int iLvl = eMiniteLvl;
					for (; iLvl != TimerLvlNUM; ++iLvl)
					{
						auto& lvlIndex = m_SlotIndex[iLvl];
						auto& lvlTasks = m_kTimerData[iLvl][lvlIndex];
						for (auto& e : lvlTasks)
						{
							AddNewTimer(e, false, iLvl);
						}
						lvlTasks.clear();
						++lvlIndex;
						if (lvlIndex >= sg_TimerSlotLimit[iLvl])
						{
							lvlIndex = 0;
							continue;
						}
						break;
					}
				}

			}

			return true;
		}

		bool AddNewTimer(Wheelement* pkWheel, bool bTriggerRightNow, int lvl = eSecondLvl)
		{
			if (pkWheel->GetInterval() > TIMER_MAX_ACCURACY || pkWheel->GetInterval() == 0)
			{
				return false;
			}

			if (!bTriggerRightNow)
			{
				int elvl = 0;
				auto interval = (lvl == eSecondLvl) ? pkWheel->GetInterval() : ((pkWheel->GetInterval() - sg_TimerDiffLimit[lvl]) % sg_TimerRateLimit[lvl]);
				for (int i = eSecondLvl; i != TimerLvlNUM; ++i)
				{
					if (interval <= sg_TimerLimit[i])
					{
						elvl = i;
						break;
					}
				}

				auto index = (interval - sg_TimerDiffLimit[elvl] + sg_TimerRateLimit[elvl] - 1) / sg_TimerRateLimit[elvl];
				if (index > 0)
				{
					index--;
				}
				index = (index + m_SlotIndex[elvl]) % sg_TimerSlotLimit[elvl];
				m_kTimerData[elvl][index].emplace_back(pkWheel);
			}
			else
			{
				m_kTimerData[eSecondLvl][m_SlotIndex[eSecondLvl]].emplace_back(pkWheel);
			}

			return true;
		}
	private:
		TimerFunc* m_pkDispatchFunc;
		std::vector<TimerSlot> m_kTimerData[TimerLvlNUM];
		std::uint8_t m_SlotIndex[TimerLvlNUM];
		std::uint32_t m_i32LeftMs;
	};
}


