#include "../include/serializer.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace rpc {

JsonSerializer::JsonSerializer() {}

std::string JsonSerializer::serialize(const RpcRequest& request) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":\"" << escapeString(request.id) << "\",";
    oss << "\"method\":\"" << escapeString(request.method) << "\",";
    oss << "\"params\":[";
    
    for (size_t i = 0; i < request.params.size(); ++i) {
        if (i > 0) oss << ",";
        oss << anyValueToJsonString(request.params[i]);
    }
    
    oss << "],";
    oss << "\"headers\":{";
    
    bool first = true;
    for (const auto& header : request.headers) {
        if (!first) oss << ",";
        oss << "\"" << escapeString(header.first) << "\":\"" << escapeString(header.second) << "\"";
        first = false;
    }
    
    oss << "},";
    oss << "\"call_type\":" << static_cast<int>(request.call_type) << ",";
    oss << "\"timeout\":" << request.timeout.count();
    oss << "}";
    
    return oss.str();
}

bool JsonSerializer::deserialize(const std::string& data, RpcRequest& request) {
    try {
        auto json = parseJson(data);
        
        request.id = getString(json, "id");
        request.method = getString(json, "method");
        request.call_type = static_cast<CallType>(getInt(json, "call_type"));
        request.timeout = std::chrono::milliseconds(getInt(json, "timeout"));
        
        // 解析参数数组
        auto params_array = getArray(json, "params");
        request.params.clear();
        for (const auto& param : params_array) {
            request.params.push_back(jsonStringToAnyValue(param));
        }
        
        // 解析头部
        auto headers_obj = getObject(json, "headers");
        request.headers.clear();
        for (const auto& header : headers_obj) {
            request.headers[header.first] = header.second;
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string JsonSerializer::serialize(const RpcResponse& response) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":\"" << escapeString(response.id) << "\",";
    
    if (response.result.has_value()) {
        oss << "\"result\":" << anyValueToJsonString(response.result) << ",";
    } else {
        oss << "\"result\":null,";
    }
    
    oss << "\"error_code\":" << static_cast<int>(response.error_code) << ",";
    oss << "\"error_message\":\"" << escapeString(response.error_message) << "\",";
    oss << "\"headers\":{";
    
    bool first = true;
    for (const auto& header : response.headers) {
        if (!first) oss << ",";
        oss << "\"" << escapeString(header.first) << "\":\"" << escapeString(header.second) << "\"";
        first = false;
    }
    
    oss << "}";
    oss << "}";
    
    return oss.str();
}

bool JsonSerializer::deserialize(const std::string& data, RpcResponse& response) {
    try {
        auto json = parseJson(data);
        
        response.id = getString(json, "id");
        response.error_code = static_cast<ErrorCode>(getInt(json, "error_code"));
        response.error_message = getString(json, "error_message");
        
        // 解析结果
        if (hasKey(json, "result") && json.at("result") != "null") {
            response.result = jsonStringToAnyValue(json.at("result"));
        }
        
        // 解析头部
        auto headers_obj = getObject(json, "headers");
        response.headers.clear();
        for (const auto& header : headers_obj) {
            response.headers[header.first] = header.second;
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string JsonSerializer::anyValueToJsonString(const AnyValue& value) {
    if (!value.has_value()) {
        return "null";
    }
    
    // 尝试不同的类型转换
    try {
        // 整数类型
        #if HAS_STD_ANY
        if (auto int_val = std::any_cast<int>(&value)) {
            return std::to_string(*int_val);
        }
        if (auto long_val = std::any_cast<long>(&value)) {
            return std::to_string(*long_val);
        }
        if (auto double_val = std::any_cast<double>(&value)) {
            return std::to_string(*double_val);
        }
        if (auto bool_val = std::any_cast<bool>(&value)) {
            return *bool_val ? "true" : "false";
        }
        if (auto str_val = std::any_cast<std::string>(&value)) {
            return "\"" + escapeString(*str_val) + "\"";
        }
        #else
        // 简化版本：假设所有值都可以转为字符串
        try {
            auto str_val = value.cast<std::string>();
            return "\"" + escapeString(str_val) + "\"";
        } catch (...) {
            try {
                auto int_val = value.cast<int>();
                return std::to_string(int_val);
            } catch (...) {
                try {
                    auto double_val = value.cast<double>();
                    return std::to_string(double_val);
                } catch (...) {
                    try {
                        auto bool_val = value.cast<bool>();
                        return bool_val ? "true" : "false";
                    } catch (...) {
                        return "null";
                    }
                }
            }
        }
        #endif
    } catch (...) {
        return "null";
    }
    
    return "null";
}

AnyValue JsonSerializer::jsonStringToAnyValue(const std::string& json_str) {
    std::string trimmed = trim(json_str);
    
    if (trimmed == "null") {
        return AnyValue();
    }
    
    if (trimmed == "true") {
        return AnyValue(true);
    }
    
    if (trimmed == "false") {
        return AnyValue(false);
    }
    
    // 字符串类型
    if (trimmed.front() == '"' && trimmed.back() == '"') {
        return AnyValue(unescapeString(trimmed.substr(1, trimmed.length() - 2)));
    }
    
    // 数字类型
    if (trimmed.find('.') != std::string::npos) {
        try {
            double val = std::stod(trimmed);
            return AnyValue(static_cast<double>(val));
        } catch (...) {}
    } else {
        try {
            int val = std::stoi(trimmed);
            return AnyValue(static_cast<int>(val));
        } catch (...) {}
    }
    
    // 默认返回字符串
    return AnyValue(std::string(trimmed));
}

// 辅助函数实现
std::string JsonSerializer::escapeString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string JsonSerializer::unescapeString(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': result += '"'; i++; break;
                case '\\': result += '\\'; i++; break;
                case 'n': result += '\n'; i++; break;
                case 'r': result += '\r'; i++; break;
                case 't': result += '\t'; i++; break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string JsonSerializer::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// 简化的JSON解析器
std::map<std::string, std::string> JsonSerializer::parseJson(const std::string& json) {
    std::map<std::string, std::string> result;
    
    // 移除外层大括号
    std::string content = trim(json);
    if (content.front() == '{' && content.back() == '}') {
        content = content.substr(1, content.length() - 2);
    }
    
    // 简单的键值对解析
    size_t pos = 0;
    while (pos < content.length()) {
        // 跳过空白字符
        while (pos < content.length() && std::isspace(content[pos])) pos++;
        if (pos >= content.length()) break;
        
        // 查找键
        if (content[pos] != '"') break;
        size_t key_start = pos + 1;
        size_t key_end = content.find('"', key_start);
        if (key_end == std::string::npos) break;
        
        std::string key = content.substr(key_start, key_end - key_start);
        pos = key_end + 1;
        
        // 跳过冒号
        while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ':')) pos++;
        if (pos >= content.length()) break;
        
        // 查找值
        size_t value_start = pos;
        size_t value_end = findValueEnd(content, pos);
        
        std::string value = trim(content.substr(value_start, value_end - value_start));
        result[key] = value;
        
        pos = value_end;
        // 跳过逗号
        while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ',')) pos++;
    }
    
    return result;
}

size_t JsonSerializer::findValueEnd(const std::string& content, size_t start) {
    size_t pos = start;
    int brace_count = 0;
    int bracket_count = 0;
    bool in_string = false;
    
    while (pos < content.length()) {
        char c = content[pos];
        
        if (!in_string) {
            if (c == '"') {
                in_string = true;
            } else if (c == '{') {
                brace_count++;
            } else if (c == '}') {
                brace_count--;
            } else if (c == '[') {
                bracket_count++;
            } else if (c == ']') {
                bracket_count--;
            } else if (c == ',' && brace_count == 0 && bracket_count == 0) {
                break;
            }
        } else {
            if (c == '"' && (pos == 0 || content[pos - 1] != '\\')) {
                in_string = false;
            }
        }
        
        pos++;
    }
    
    return pos;
}

std::string JsonSerializer::getString(const std::map<std::string, std::string>& json, const std::string& key) {
    auto it = json.find(key);
    if (it != json.end()) {
        std::string value = trim(it->second);
        if (value.front() == '"' && value.back() == '"') {
            return unescapeString(value.substr(1, value.length() - 2));
        }
        return value;
    }
    return "";
}

int JsonSerializer::getInt(const std::map<std::string, std::string>& json, const std::string& key) {
    auto it = json.find(key);
    if (it != json.end()) {
        try {
            return std::stoi(trim(it->second));
        } catch (...) {}
    }
    return 0;
}

std::vector<std::string> JsonSerializer::getArray(const std::map<std::string, std::string>& json, const std::string& key) {
    std::vector<std::string> result;
    auto it = json.find(key);
    if (it != json.end()) {
        std::string array_str = trim(it->second);
        if (array_str.front() == '[' && array_str.back() == ']') {
            std::string content = array_str.substr(1, array_str.length() - 2);
            // 简单的数组解析
            size_t pos = 0;
            while (pos < content.length()) {
                size_t end = findValueEnd(content, pos);
                result.push_back(trim(content.substr(pos, end - pos)));
                pos = end;
                while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ',')) pos++;
            }
        }
    }
    return result;
}

std::map<std::string, std::string> JsonSerializer::getObject(const std::map<std::string, std::string>& json, const std::string& key) {
    auto it = json.find(key);
    if (it != json.end()) {
        return parseJson(it->second);
    }
    return {};
}

bool JsonSerializer::hasKey(const std::map<std::string, std::string>& json, const std::string& key) {
    return json.find(key) != json.end();
}

} // namespace rpc 