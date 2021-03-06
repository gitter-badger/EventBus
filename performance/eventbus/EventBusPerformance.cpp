//
// Created by Dawid Drozd aka Gelldur on 05.08.17.
//
#include <random>

#include <benchmark/benchmark.h>
#include <eventbus/EventBus.h>

namespace
{

void checkNListeners(benchmark::State& state, const int listenersCount)
{
	Dexode::EventBus bus;
	int sum = 0;

	Dexode::Event<int> simple{"simple"};
	for (int i = 0; i < listenersCount; ++i)
	{
		bus.listen(simple, [&](int value)
		{
			benchmark::DoNotOptimize(sum += value * 2);
		});
	}

	while (state.KeepRunning())//Performance area!
	{
		bus.notify(simple, 2);
	}
	state.counters["sum"] = sum;
}

void checkSimpleNotification(benchmark::State& state)
{
	checkNListeners(state, 1);
}

void check10Listeners(benchmark::State& state)
{
	checkNListeners(state, 10);
}

void check100Listeners(benchmark::State& state)
{
	checkNListeners(state, 100);
}

void check1kListeners(benchmark::State& state)
{
	checkNListeners(state, 1000);
}

void call1kLambdas_compare(benchmark::State& state)
{
	int sum = 0;
	std::vector<std::function<void(int)>> callbacks;
	callbacks.reserve(1000);
	for (int i = 0; i < 1000; ++i)
	{
		callbacks.emplace_back([&](int value)
							   {
								   benchmark::DoNotOptimize(sum += value * 2);
							   });
	}

	while (state.KeepRunning())//Performance area!
	{
		for (int i = 0; i < 1000; ++i)
			//		for (auto& callback :callbacks)
		{
			callbacks[i](2);
		}
	}
	state.counters["sum"] = sum;
}

void checkNNotificationsForNListeners(benchmark::State& state, const int notificationsCount, const int listenersCount)
{
	std::mt19937 generator(311281);
	std::uniform_int_distribution<int> uniformDistribution(0, notificationsCount - 1);

	//We generate here N different notifications
	std::vector<Dexode::Event<int>> notifications;
	notifications.reserve(notificationsCount);
	for (int i = 0; i < notificationsCount; ++i)
	{
		notifications.emplace_back(std::string{"event_"} + std::to_string(i));
	}

	Dexode::EventBus bus;
	int sum = 0;
	for (int i = 0; i < listenersCount; ++i)//We register M listeners for N notifications using uniform distribution
	{
		const auto& notification = notifications.at(uniformDistribution(generator));
		bus.listen(notification, [&](int value)
		{
			benchmark::DoNotOptimize(sum += value * 2);//we use it to prevent some? optimizations
		});
	}

	while (state.KeepRunning())//Performance area!
	{
		//Pick random notification
		const auto& notification = notifications.at(uniformDistribution(generator));
		bus.notify(notification, uniformDistribution(generator));
	}
	state.counters["sum"] = sum;
}

void check10NotificationsFor1kListeners(benchmark::State& state)
{
	checkNNotificationsForNListeners(state, 10, 1000);
}

void check100NotificationsFor1kListeners(benchmark::State& state)
{
	checkNNotificationsForNListeners(state, 100, 1000);
}

void check1kNotificationsFor1kListeners(benchmark::State& state)
{
	checkNNotificationsForNListeners(state, 1000, 1000);
}

void check100NotificationsFor10kListeners(benchmark::State& state)
{
	checkNNotificationsForNListeners(state, 100, 10000);
}

void checkNotifyFor10kListenersWhenNoOneListens(benchmark::State& state)
{
	Dexode::EventBus bus;
	int sum = 0;
	Dexode::Event<int> simple{"simple"};
	Dexode::Event<int> unknown{"unknown"};
	for (int i = 0; i < 10000; ++i)
	{
		bus.listen(simple, [&](int value)
		{
			benchmark::DoNotOptimize(sum += value * 2);
		});
	}

	while (state.KeepRunning())//Performance area!
	{
		bus.notify(unknown, 2);
	}
	state.counters["sum"] = sum;
}

}

BENCHMARK(call1kLambdas_compare);

BENCHMARK(checkSimpleNotification);
BENCHMARK(check10Listeners);
BENCHMARK(check100Listeners);
BENCHMARK(check1kListeners);
BENCHMARK(check10NotificationsFor1kListeners);
BENCHMARK(check100NotificationsFor1kListeners);
BENCHMARK(check1kNotificationsFor1kListeners);
BENCHMARK(check100NotificationsFor10kListeners);
BENCHMARK(checkNotifyFor10kListenersWhenNoOneListens);

BENCHMARK_MAIN();
