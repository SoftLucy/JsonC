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
    auto jsonOpt = Json::tryParse(
        LR"({"network": true, "port": 8080, "sensorName": "Temperature"})"
    );
    if (!jsonOpt) return 1;
    auto json = jsonOpt.value();

    auto objectOpt = Json::try_if_JsonObject(json);
    if (!objectOpt) return 1;
    auto object = objectOpt.value();
    auto map = object.m;

    auto networkOpt = Json::try_if_bool(map[L"network"]);
    auto portOpt = Json::try_if_Number(map[L"port"]);
    auto nameOpt = Json::try_if_wstring(map[L"sensorName"]);
    if (!networkOpt || !portOpt || !nameOpt) return 1;

    bool network = networkOpt.value();
    long long port = portOpt.value();
    std::wstring name = nameOpt.value();

    std::wcout << L"Network: " << network << L'\n';
    std::wcout << L"Port: " << port << L'\n';
    std::wcout << L"Sensor: " << name << L'\n';
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
