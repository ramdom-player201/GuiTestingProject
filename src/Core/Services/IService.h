#pragma once

class IService {
protected:
	IService() = default; // prevent direct instanciation
	IService(const IService&) = delete;
	IService(IService&&) = delete;
	IService& operator = (const IService&) = delete;
	IService& operator = (IService&&) = delete;
public:
	virtual ~IService() = default; // virtual destructor

	// common service methods
	// // virtual void initialise() = 0;
	// // virtual void shutdown() = 0;
};