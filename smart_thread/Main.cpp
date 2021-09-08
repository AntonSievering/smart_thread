#include "smart_thread.h"

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
