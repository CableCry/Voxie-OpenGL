#pragma once
#include <atomic>
#include <cstdint>
#include <expected>
#include <format>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

class Log {
  public:
    Log() = delete; // never instantiated

    enum class Level : std::uint8_t { Trace, Info, Warn, Error, Off };

    static void setLevel(Level l) noexcept {
        level_.store(l, std::memory_order_relaxed);
    }
    [[nodiscard]] static auto level() noexcept -> Level {
        return level_.load(std::memory_order_relaxed);
    }

    template <typename... Args> static void trace(std::format_string<Args...> f, Args&&... a) {
        write(Level::Trace, "trace", std::cout, std::format(f, std::forward<Args>(a)...));
    }
    template <typename... Args> static void info(std::format_string<Args...> f, Args&&... a) {
        write(Level::Info, "info", std::cout, std::format(f, std::forward<Args>(a)...));
    }
    template <typename... Args> static void ok(std::format_string<Args...> f, Args&&... a) {
        write(Level::Info, "ok", std::cout, std::format(f, std::forward<Args>(a)...));
    }
    template <typename... Args> static void gpu(std::format_string<Args...> f, Args&&... a) {
        write(Level::Info, "gpu", std::cout, std::format(f, std::forward<Args>(a)...));
    }
    template <typename... Args> static void warn(std::format_string<Args...> f, Args&&... a) {
        write(Level::Warn, "warn", std::cerr, std::format(f, std::forward<Args>(a)...));
    }
    template <typename... Args> static void error(std::format_string<Args...> f, Args&&... a) {
        write(Level::Error, "ERROR", std::cerr, std::format(f, std::forward<Args>(a)...));
    }

    static void section(std::string_view title) {
        if (Level::Info < level()) {
            return;
        }
        std::scoped_lock lock{mutex()};
        std::cout << "\n--- " << title << " ---\n";
    }

    [[nodiscard]] static auto bytes(std::uint64_t n) -> std::string {
        constexpr std::uint64_t KiB = 1ULL << 10;
        constexpr std::uint64_t MiB = 1ULL << 20;
        constexpr std::uint64_t GiB = 1ULL << 30;
        if (n >= GiB) {
            return std::format("{:.2f} GiB", static_cast<double>(n) / GiB);
        }
        if (n >= MiB) {
            return std::format("{:.2f} MiB", static_cast<double>(n) / MiB);
        }
        if (n >= KiB) {
            return std::format("{:.2f} KiB", static_cast<double>(n) / KiB);
        }
        return std::format("{} B", n);
    }

  private:
    static void write(Level lvl, std::string_view tag, std::ostream& os, std::string_view msg) {
        if (lvl < level()) {
            return;
        }
        std::scoped_lock lock{mutex()};
        // Flush every line: a buffered stdout loses exactly the messages that
        // precede a crash or a kill, which are the ones worth having.
        // ponytail: revisit only if per-frame logging ever shows up in a profile.
        os << std::format("[{:^5}] {}\n", tag, msg) << std::flush;
    }

    static auto mutex() -> std::mutex& {
        static std::mutex m; // function-local: no static-init-order issues
        return m;
    }

    inline static std::atomic<Level> level_ = Level::Info;
};

template <typename T, typename E>
[[nodiscard]] auto orDieExp(std::expected<T, E>&& e, std::string_view what) -> T {
    if (!e) {
        Log::error("{}: {}", what, e.error());
        throw std::runtime_error{std::string{what}};
    }
    return std::move(*e);
}
