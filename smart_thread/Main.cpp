#include <thread>
#include <memory>
#include <chrono>
#include <iostream>

class stop_token
{
	friend class smart_thread;

private:
	std::shared_ptr<bool> m_pSharedToken = std::make_shared<bool>(false);

public:
	stop_token() noexcept {}

private:
	bool &get() noexcept
	{
		return *m_pSharedToken.get();
	}

	const bool &get() const noexcept
	{
		return *m_pSharedToken.get();
	}

	void request_stop() noexcept
	{
		get() = true;
	}

public:
	[[nodiscard]] bool stop_requested() const noexcept
	{
		return get();
	}
};

class smart_thread
{
private:
	std::shared_ptr<std::thread> m_pThread;
	stop_token m_threadStopToken{};

public:
	smart_thread() noexcept = default;

	template <class Fn, class ... Args>
	smart_thread(Fn &&function, Args && ... args) noexcept
	{
		m_pThread = std::make_shared<std::thread>(function, m_threadStopToken, args...);
	}

	~smart_thread() noexcept
	{
		if (joinable())
		{
			request_stop();
			join();
		}
	}

private:
	std::thread &get() noexcept
	{
		return *m_pThread.get();
	}

	const std::thread &get() const noexcept
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
		m_threadStopToken.request_stop();
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

void k(stop_token st)
{
	std::cout << "something\n";
}

int main()
{
	smart_thread thread;
	thread = smart_thread(k);
}
