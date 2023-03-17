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


constexpr int ThreadCount = 8;
volatile int Loop = 10000000 / ThreadCount;


using namespace DirectX;

using TestMap = std::unordered_map<int, std::string>;
using TestMap2 = concurrency::concurrent_unordered_map<int, std::string>;

TestMap a;
TestMap2 b;


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
					TestMap localMap;
					for (int j{ Loop * threadNumber }; j < Loop * (threadNumber + 1); ++j)
					{
						/*
						//compare with this ^_T
						int asd = aasd.load();
						a[asd] = std::to_string(j) + "asd";
						++aasd;
						*/
						localMap[j] = std::to_string(j) + "asd";
					}
					
					lock.lock();
					a.merge(std::move(localMap));
					lock.unlock();
				});
		}
		std::for_each(threads.begin(), threads.end(), [](auto& thread) { thread.join(); });

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		//reset
		//check size equal to Total Loop
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
						/*
						//compare with this ^_T
						int asd = aasd.load();
						a[asd] = std::to_string(j) + "asd";
						++aasd;
						*/
						b[aasd++] = std::to_string(j) + "asd";
					}
				});
		}
		std::for_each(threads.begin(), threads.end(), [](auto& thread) { thread.join(); });

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		//reset
		b.clear();
	}


	{
		auto begin = std::chrono::high_resolution_clock::now();

		concurrency::parallel_for(0, Loop * ThreadCount, [](int i)
			{
				//std::lock_guard<std::mutex> l(lock);
				b[i] = std::to_string(i) + "asd";
			});

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

		//reset
		b.clear();
	}
}
