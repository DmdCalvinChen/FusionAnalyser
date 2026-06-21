#include <iostream>
#include <string>
#include <map>
#include "MeshExt/AbsMesh.h"
#include "data/separationmanager.h"
#include "common/ml_mesh_type.h"
#include <wrap/io_trimesh/import_stl.h>

// 这是一个 C++ 后端命令行工具的伪代码/骨架
// 你需要将其集成到你的 Qt 项目中，并链接 SeparationManager 和 DentalFeatures。

int main(int argc, char* argv[]) {
    std::string upper_stl_path = "";
    std::string lower_stl_path = "";

    // 1. 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--upper" && i + 1 < argc) {
            upper_stl_path = argv[++i];
        } else if (arg == "--lower" && i + 1 < argc) {
            lower_stl_path = argv[++i];
        }
    }

    // 测试真实的 C++ 接口实例化 (不执行实际的 loadMesh)
    // 以确保能够找到共享库及其符号
    SeparationManager upperManager;
    SeparationManager lowerManager;

    // 3. 输出纯 JSON (这里用伪造数据演示 JSON 结构)
    std::cout << "{\n";
    std::cout << "  \"upper_widths\": {\n";
    std::cout << "    \"11\": 8.5, \"12\": 7.0, \"13\": 8.0, \"14\": 7.5, \"15\": 7.0, \"16\": 10.5,\n";
    std::cout << "    \"21\": 8.5, \"22\": 7.0, \"23\": 8.0, \"24\": 7.5, \"25\": 7.0, \"26\": 10.5\n";
    std::cout << "  },\n";
    std::cout << "  \"lower_widths\": {\n";
    std::cout << "    \"31\": 5.5, \"32\": 6.0, \"33\": 7.0, \"34\": 7.5, \"35\": 7.5, \"36\": 11.0,\n";
    std::cout << "    \"41\": 5.5, \"42\": 6.0, \"43\": 7.0, \"44\": 7.5, \"45\": 7.5, \"46\": 11.0\n";
    std::cout << "  },\n";
    std::cout << "  \"status\": \"success\",\n";
    std::cout << "  \"message\": \"SeparationManager successfully instantiated!\"\n";
    std::cout << "}\n";

    return 0;
}
