/*
    Utility functions shared by output generators
 */
#include "util.h"
#include "fmt/format.h"
#include "pystring.h"

namespace shdc {
namespace util {

ErrMsg check_errors(const Input& inp,
                      const Spirvcross& spirvcross,
                      Slang::Enum slang)
{
    for (const auto& item: inp.programs) {
        const Program& prog = item.second;
        int vs_snippet_index = inp.snippet_map.at(prog.vs_name);
        int fs_snippet_index = inp.snippet_map.at(prog.fs_name);
        int vs_src_index = spirvcross.find_source_by_snippet_index(vs_snippet_index);
        int fs_src_index = spirvcross.find_source_by_snippet_index(fs_snippet_index);
        if (vs_src_index < 0) {
            return inp.error(inp.snippets[vs_snippet_index].lines[0],
                fmt::format("no generated '{}' source for vertex shader '{}' in program '{}'",
                    Slang::to_str(slang), prog.vs_name, prog.name));
        }
        if (fs_src_index < 0) {
            return inp.error(inp.snippets[vs_snippet_index].lines[0],
                fmt::format("no generated '{}' source for fragment shader '{}' in program '{}'",
                    Slang::to_str(slang), prog.fs_name, prog.name));
        }
    }
    // all ok
    return ErrMsg();
}

const char* uniform_type_str(Uniform::type_t type) {
    switch (type) {
        case Uniform::FLOAT: return "float";
        case Uniform::FLOAT2: return "vec2";
        case Uniform::FLOAT3: return "vec3";
        case Uniform::FLOAT4: return "vec4";
        case Uniform::MAT4: return "mat4";
        default: return "FIXME";
    }
}

int uniform_size(Uniform::type_t type, int array_size) {
    if (array_size > 1) {
        switch (type) {
            case Uniform::FLOAT4:
            case Uniform::INT4:
                return 16 * array_size;
            case Uniform::MAT4:
                return 64 * array_size;
            default: return 0;
        }
    }
    else {
        switch (type) {
            case Uniform::FLOAT:
            case Uniform::INT:
                return 4;
            case Uniform::FLOAT2:
            case Uniform::INT2:
                return 8;
            case Uniform::FLOAT3:
            case Uniform::INT3:
                return 12;
            case Uniform::FLOAT4:
            case Uniform::INT4:
                return 16;
            case Uniform::MAT4:
                return 64;
            default: return 0;
        }
    }
}

int roundup(int val, int round_to) {
    return (val + (round_to - 1)) & ~(round_to - 1);
}

std::string mod_prefix(const Input& inp) {
    if (inp.module.empty()) {
        return "";
    }
    else {
        return fmt::format("{}_", inp.module);
    }
}

const SpirvcrossSource* find_spirvcross_source_by_shader_name(const std::string& shader_name, const Input& inp, const Spirvcross& spirvcross) {
    assert(!shader_name.empty());
    int snippet_index = inp.snippet_map.at(shader_name);
    int src_index = spirvcross.find_source_by_snippet_index(snippet_index);
    if (src_index >= 0) {
        return &spirvcross.sources[src_index];
    }
    else {
        return nullptr;
    }
}

const BytecodeBlob* find_bytecode_blob_by_shader_name(const std::string& shader_name, const Input& inp, const Bytecode& bytecode) {
    assert(!shader_name.empty());
    int snippet_index = inp.snippet_map.at(shader_name);
    int blob_index = bytecode.find_blob_by_snippet_index(snippet_index);
    if (blob_index >= 0) {
        return &bytecode.blobs[blob_index];
    }
    else {
        return nullptr;
    }
}

std::string to_pascal_case(const std::string& str) {
    std::vector<std::string> splits;
    pystring::split(str, splits, "_");
    std::vector<std::string> parts;
    for (const auto& part: splits) {
        parts.push_back(pystring::capitalize(part));
    }
    return pystring::join("", parts);
}

std::string to_ada_case(const std::string& str) {
    std::vector<std::string> splits;
    pystring::split(str, splits, "_");
    std::vector<std::string> parts;
    for (const auto& part: splits) {
        parts.push_back(pystring::capitalize(part));
    }
    return pystring::join("_", parts);
}

std::string to_camel_case(const std::string& str) {
    std::string res = to_pascal_case(str);
    res[0] = tolower(res[0]);
    return res;
}

std::string to_upper_case(const std::string& str) {
    return pystring::upper(str);
}

std::string replace_C_comment_tokens(const std::string& str) {
    static const std::string comment_start_old = "/*";
    static const std::string comment_start_new = "/_";
    static const std::string comment_end_old = "*/";
    static const std::string comment_end_new = "_/";
    std::string s = pystring::replace(str, comment_start_old, comment_start_new);
    s = pystring::replace(s, comment_end_old, comment_end_new);
    return s;
}

const char* slang_file_extension(Slang::Enum c, bool binary) {
    switch (c) {
        case Slang::GLSL410:
        case Slang::GLSL430:
        case Slang::GLSL300ES:
            return ".glsl";
        case Slang::HLSL4:
        case Slang::HLSL5:
            return binary ? ".fxc" : ".hlsl";
        case Slang::METAL_MACOS:
        case Slang::METAL_IOS:
        case Slang::METAL_SIM:
            return binary ? ".metallib" : ".metal";
        case Slang::WGSL:
            return ".wgsl";
        default:
            return "";
    }
}

} // namespace util
} // namespace shdc
