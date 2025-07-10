#pragma once

#include <map>
#include <mutex>
#include "Services/IService.h"

//services
#include "Services/CommandConsole.h"
#include "Services/WindowManager.h"
#include "Services/ObjectManager.h"

enum class ServiceType {
	CommandConsole,
	WindowManager,
	ObjectManager
};

class Services {
private:
	// define instance list and mtx
	inline static std::mutex mtx; // for multi-thread locking
	inline static std::map<ServiceType, std::unique_ptr<IService>> instances;

	Services() = delete; // prevent instanciation

	static IService* createService(ServiceType service) {
		switch (service) {
		case ServiceType::CommandConsole: {
			return new CommandConsole();
		}
		case ServiceType::WindowManager: {
			return new WindowManager();
		}
		case ServiceType::ObjectManager: {
			return new ObjectManager();
		}
		default:
			throw std::invalid_argument("Unknown service type");
		}
	}

public:
	static IService* getService(ServiceType type) {
		auto& instance = instances[type];

		// double-checked locking (for thread-safety or something)
		if (!instance) {
			std::lock_guard<std::mutex> lock(mtx); // lock is removed when lock exits scope
			if (!instance) {
				instance = std::unique_ptr<IService>(createService(type));
			}
		}
		return instance.get();
	}

	static void initialise() {
		// register services with their implementations
		instances[ServiceType::CommandConsole] = nullptr;
		instances[ServiceType::WindowManager] = nullptr;
	}
};