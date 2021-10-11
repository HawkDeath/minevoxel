#pragma once

#include <spdlog/spdlog.h>
#include <stdexcept>

#define RT_THROW(...) throw std::runtime_error(__VA_ARGS__)
#define UNUSE(val) (void)val

#define LOGGER

#ifdef LOGGER
#define LOG(...) do {spdlog::info(__VA_ARGS__);} while(0)
#define WLOG(...) do {spdlog::warn(__VA_ARGS__);} while(0)
#define ELOG(...) do {spdlog::error(__VA_ARGS__);} while(0)

#else
#define LOG(...)
#define WLOG(...)
#define ELOG(...)
#endif // LOGGER

