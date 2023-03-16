#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <ppl.h>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <string>
#include <unordered_map>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>

constexpr int ThreadCount = 4;
volatile int Loop = 1000000 / ThreadCount;

using namespace DirectX;

using TestMap = concurrency::concurrent_unordered_map<int, std::string>;
//using TestMap = std::unordered_map<int, std::string>;
TestMap a;

std::mutex lock;

std::atomic_int aasd;

int main()
{
	{
		auto begin = std::chrono::high_resolution_clock::now();

		std::vector<std::thread> threads;
		for (int i{}; i < ThreadCount; ++i)
		{
			threads.emplace_back([threadNumber = i]()
				{
					for (int j{ Loop * threadNumber }; j < Loop * (threadNumber + 1); ++j)
					{
						/*
						//compare with this ^_T
						int asd = aasd.load();
						a[asd] = std::to_string(j) + "asd";
						++aasd;
						*/
						a[aasd++] = std::to_string(j) + "asd";
					}
				});
		}
		std::for_each(threads.begin(), threads.end(), [](auto& thread) { thread.join(); });

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		//reset
		a.clear();
	}

	{
		auto begin = std::chrono::high_resolution_clock::now();

		std::vector<std::thread> threads;
		for (int i{}; i < ThreadCount; ++i)
		{
			threads.emplace_back([threadNumber = i]()
				{
					for (int j{ Loop * threadNumber }; j < Loop * (threadNumber + 1); ++j)
					{
						//volatile useless maybe
						volatile std::string asd = a[j];
					}
				});
		}
		std::for_each(threads.begin(), threads.end(), [](auto& thread) { thread.join(); });

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		//reset
		a.clear();
	}

	{
		auto begin = std::chrono::high_resolution_clock::now();
		concurrency::parallel_for(0, Loop * ThreadCount, [](int i)
			{
				//std::lock_guard<std::mutex> l(lock);
				a[i] = std::to_string(i) + "asd";
			});

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		//reset
		a.clear();
	}
}
