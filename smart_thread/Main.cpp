#include <thread>
#include <memory>
#include <chrono>
#include <iostream>

class stop_token
{
	friend class smart_threads;

private:
	const bool *m_pStopRequested = nullptr;

public:
	stop_token() noexcept = default;

	stop_token(const bool *pStopRequested)
	{
		m_pStopRequested = pStopRequested;
	}

public:
	bool stop_requested() const noexcept
	{
		return *m_pStopRequested;
	}
};

class smart_thread
{
private:
	class internal_thread
	{
		friend class smart_thread;

	private:
		std::thread m_thread;
		bool        m_bRequestStop = false;

	public:
		internal_thread() noexcept = default;

		template <class Fn, class ... Args>
		internal_thread(Fn &&function, Args && ... args) noexcept
		{
			m_thread = std::thread(function, stop_token(&m_bRequestStop), args...);
		}

		~internal_thread() noexcept
		{
			if (joinable())
			{
				request_stop();
				join();
			}
		}

	private:
		[[nodiscard]] bool joinable() const noexcept
		{
			return m_thread.joinable();
		}

		void request_stop() noexcept
		{
			m_bRequestStop = true;
		}

		void join() noexcept
		{
			m_thread.join();
		}

		void detach() noexcept
		{
			m_thread.detach();
		}

		[[nodiscard]] std::thread::id get_id() noexcept
		{
			return m_thread.get_id();
		}
	};
	
private:
	std::shared_ptr<internal_thread> m_pThread;

public:
	smart_thread() noexcept = default;

	template <class Fn, class ... Args>
	smart_thread(Fn &&function, Args && ... args) noexcept
	{
		m_pThread = std::make_shared<internal_thread>(function, args...);
	}
	
private:
	internal_thread &get() noexcept
	{
		return *m_pThread.get();
	}

	const internal_thread &get() const noexcept
	{
		return *m_pThread.get();
	}

public:
	bool thread_created() const noexcept
	{
		return m_pThread.get() != nullptr;
	}

	void request_stop() noexcept
	{
		get().request_stop();
	}

	[[nodiscard]] bool joinable() const noexcept
	{
		return get().joinable();
	}

	void join() noexcept
	{
		get().join();
	}

	void detach() noexcept
	{
		get().detach();
	}

	[[nodiscard]] std::thread::id get_id() noexcept
	{
		return get().get_id();
	}
};

void k(stop_token st, int i)
{
	while (!st.stop_requested())
		std::cout << i << std::endl;
	std::cout << "end\n";
}

int main()
{
	smart_thread thread;
	thread = smart_thread(k, 1);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	thread = smart_thread(k, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
