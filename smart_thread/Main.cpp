#include <iostream>
#include <thread>

class stop_token
{
	friend class smart_thread;
	friend class member_thread;

private:
	bool *m_pStopRequested = nullptr;

private:
	stop_token() noexcept = default;

	stop_token(bool *pStopRequested) noexcept
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
protected:
	std::thread m_thread;
	bool *m_pStopRequested = nullptr;

public:
	smart_thread() noexcept = default;

	template <class Fn, class ... Args>
	smart_thread(Fn &&fn, Args && ... args) noexcept
	{
		m_pStopRequested = createMemory();
		m_thread = std::thread(fn, stop_token(m_pStopRequested), args ...);
	}

	smart_thread(const smart_thread &) noexcept = delete;

	smart_thread(smart_thread &&rhs) noexcept
	{
		move_over(std::forward<smart_thread &&>(rhs));
	}

	virtual ~smart_thread() noexcept
	{
		terminate_self();
	}

public:
	smart_thread &operator=(const smart_thread &) noexcept = delete;

	smart_thread &operator=(smart_thread &&rhs) noexcept
	{
		terminate_self();
		move_over(std::forward<smart_thread &&>(rhs));
		return *this;
	}

private:
	void deleteMemory() noexcept
	{
		delete m_pStopRequested;
		m_pStopRequested = nullptr;
	}

protected:
	static bool *createMemory() noexcept
	{
		bool *mem = new bool();
		*mem = false;
		return mem;
	}

	void move_over(smart_thread &&rhs) noexcept
	{
		m_thread = std::move(rhs.m_thread);
		m_pStopRequested = rhs.m_pStopRequested;
		rhs.m_pStopRequested = nullptr;
	}

	void terminate_self() noexcept
	{
		if (m_pStopRequested)
		{
			request_stop();
			if (joinable())
				m_thread.join();
			deleteMemory();
		}
	}

public:
	bool joinable() const noexcept
	{
		return m_thread.joinable();
	}

	void join() noexcept
	{
		m_thread.join();
	}

	void detach() noexcept
	{
		m_thread.detach();
	}

	std::thread::id get_id() const noexcept
	{
		return m_thread.get_id();
	}

	void request_stop() noexcept
	{
		*m_pStopRequested = true;
	}
};

class member_thread : public smart_thread
{
public:
	member_thread() noexcept = default;

	template<class Fn, class T, class ... Args>
	member_thread(Fn &&fn, T *obj, Args ... args) noexcept
	{
		m_pStopRequested = createMemory();
		m_thread = std::thread(fn, obj, stop_token(m_pStopRequested), args ...);
	}

	member_thread(const member_thread &) noexcept = delete;

	member_thread(member_thread &&rhs) noexcept
	{
		move_over(std::forward<member_thread&&>(rhs));
	}

public:
	member_thread &operator=(const member_thread &) noexcept = default;

	member_thread &operator=(member_thread &&rhs) noexcept
	{
		terminate_self();
		move_over(std::forward<member_thread&&>(rhs));
	}
};

void k(stop_token token, int j, int k)
{
	while (!token.stop_requested())
		std::cout << j + k++ << std::endl;
	std::cout << "end\n";
}

struct l
{
	void fn(stop_token token, int i)
	{
		while (!token.stop_requested())
			std::cout << i++ << std::endl;
		std::cout << "end\n";
	}
};

int main()
{
	smart_thread thread = smart_thread(k, 1, 2);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	l x;
	thread = member_thread(&l::fn, &x, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
