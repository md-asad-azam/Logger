# Logger

A lightweight C++ logger that supports configurable log levels, optional file output, and asynchronous queue-based logging on a worker thread.

## Recent fixes

- Graceful logger shutdown now drains the queue and joins the worker thread explicitly (`Logger::Shutdown()`), so no `sleep` workaround is required in `main()`.
- Function/file formatting was improved by using fixed-width output and right-side truncation for long symbols.
- Logging level parsing now supports human-readable values like `trace` and `debug` from config.
- `AddLog` was updated to avoid unnecessary string copies by accepting `std::string_view`.

## Notes

- `AnyComponent.cfg` controls runtime behavior (level, filename width, thread id width, file logging, etc.).
