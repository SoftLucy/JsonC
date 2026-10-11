# JsonC

A small JSON parser and serializer for C++ in a **single header file**. It uses only the C++ standard library, so there is nothing to install or link.

```cpp
#include "Json.hpp"
```

## Features

- Single header (`Json.hpp`), no dependencies beyond the standard library (`<string>`, `<map>`, `<vector>`, `<variant>`, `<optional>`)
- Parses objects, arrays, strings, numbers, booleans and `null`
- Values are stored as a `std::variant`, so there is no custom type hierarchy
- Parse errors are reported as an empty `std::optional`, not as exceptions
- Typed accessors (`try_if_wstring`, `try_if_Number`, ...) return `std::optional` as well
- Serializes a parsed object back to JSON text (`Json::to_String`)

## Requirements

C++20 (uses `std::variant`, `std::optional` and `std::map::contains`). Tested with MSVC from Visual Studio 2019 (`/std:c++latest`).

## Example

```cpp
#include "Json.hpp"
#include <iostream>

int main() {
    std::wstring text =
        LR"({"name": "Sensor 1", "active": true, "values": [1, 2, 3]})";

    auto root = Json::tryParse(text);
    if (!root) {
        std::wcerr << L"Invalid JSON\n";
        return 1;
    }

    auto obj = Json::try_if_JsonObject(*root);
    if (!obj) return 2;
    auto& m = obj->m;

    std::wcout << *Json::try_if_wstring(m[L"name"]) << L"\n";   // Sensor 1
    std::wcout << *Json::try_if_bool(m[L"active"]) << L"\n";    // 1

    if (auto values = Json::try_if_Vector(m[L"values"]))
        for (auto& v : *values)
            std::wcout << *Json::try_if_Number(v) << L" ";      // 1 2 3
}
```

## API overview

| Function | Purpose |
|---|---|
| `Json::tryParse(std::wstring)` | Parses a document (object or array); returns `std::optional<JsonValue>` |
| `Json::parseValue`, `parseJsonObject`, `parseJsonArray`, `parseString`, `parseNumber`, `parseBool`, `parseNull` | Parsers for the individual JSON types (iterator based) |
| `Json::try_if_JsonObject`, `try_if_Vector`, `try_if_wstring`, `try_if_Number`, `try_if_float`, `try_if_bool`, `try_if_Ptr` | Access a `JsonValue` as a specific type, `std::nullopt` if it holds another type |
| `Json::to_String(JsonObject)` | Serializes an object to text |

## Design notes

- `JsonValue` is a `std::variant<JsonObject, std::vector<JsonObject>, std::wstring, long long, float, bool, void *>`. A `std::variant` cannot contain itself, so array elements are wrapped in a `JsonObject`.
- Strings are `std::wstring`.
- `null` is stored as a `void *` (`nullptr`).

## License

MIT, see [LICENSE](LICENSE).
